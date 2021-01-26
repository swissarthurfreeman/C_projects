#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA "DonneesPriveesSensibles"
#define GIGA 1073741824 //nombre d’octets dans 1 Go
#define MAXDATASIZE (7 * GIGA) //Taille de la memoire de la machine ou est exectue ce programme

void exit_err (const char* str) {
    perror(str);
    exit( EXIT_FAILURE );
}

int main(int argc , char* argv []) {

    struct rlimit myLimits;
    struct rusage myUsage;
    char *ptr;
    long int i;
    int lock = 0;

    if(( argc > 1) && (strcmp("lock", argv [1]) == 0))
        lock = 1;

    //limites en mémoire soft et max, e.g. on alloue autant que l'on veux, pas de limite.
    myLimits.rlim_cur = RLIM_INFINITY ;
    myLimits.rlim_max = RLIM_INFINITY ;

    //quantitée maximale de bytes en ram étant utilisables en ram.
    //donc on donne libre feux... (il faut être root afin de le faire, d'ou le install)
    if( setrlimit (RLIMIT_MEMLOCK , &myLimits) == -1)
        exit_err("Impossible de regler la limite RLIMIT_MEMLOCK");

    //on enlève les droits root.
    //getuid renvoie RUID, setuid change le EUID.
    if(setuid(getuid ()) == -1)
        exit_err("Impossible de regler l’uid");

    if(lock) {
        //mlockall vas mettre toutes les pages virtuelles du processus
        //en mémoire ram (évite qu'elles soient mises en swap)
        //MCL_FUTURE fait que chaque page ajoutée au processus dans son espace de mémoire virtuelle
        //sera automatiquement vérouillée en mémoire ram. 
        //https://www.gnu.org/software/libc/manual/html_node/Page-Lock-Functions.html
        if(mlockall ( MCL_FUTURE ) == -1)
            exit_err("Impossible de bloquer la memoire");
        printf("Memory locked\n");
    }

    //on alloue 8 gigas de mémoire...
    if(( ptr = malloc( MAXDATASIZE ) ) == NULL)
        exit_err("Impossible d’allouer de la memoire");

    //Initialisation des donnees (on mets tout a 0)
    for(i = 0 ; i < MAXDATASIZE ; i++)
        *( ptr+i) = 0;

    //Placement des donnees en memoire
    //la mémoire sera DATDATADATADATA...
    //c'est dans ce for qu'on vas générer
    //des défauts de page si on a pas fait de mlockall.
    for(i = 0 ; i < MAXDATASIZE ; i += sizeof(DATA))
        strcpy(ptr+i, DATA);
    
    //on récupère l'utilisation de ressources par le processus.
    if( getrusage (RUSAGE_SELF , &myUsage) == -1)
        exit_err("Impossible de lire les usages du processus");

    //myUsage.ru_majflt sont les défauts de page majeur DU PROCESSUS.
    //un défaut de page majeur a lieu quand la page n'est pas présente
    //dans la mémoire physique (ici par manque de place, elle sera dans le swap)
    printf("Number de defauts de pages majeurs: %ld\n", myUsage.ru_majflt );

    //on débloque (e.g. le kernel peut de nouveau mettre dans le swap)
    if(lock)
        if( munlockall () == -1)
            exit_err("Impossible de debloquer la memoire");

    return 0;
}

/*
1. A l’aide d’un schema expliquer la notion d’adressage virtuel et sa relation avec
la mémoire physique.

cf. Schéma dans dossier courant.

2. Que fait le programme suivant ?

Ce programme alloue un pointeur de taille 7Gios sur le tas et écrit DATADATADATA
a l'intérieur. Selon le paramètre lock, on aura des défauts de page ou pas, car
on utilise la fonction mlockall dans le processus, qui fera en sorte, avec le flag
MCL_FUTURE que toutes les pages virtuelles du processus soient présentes en RAM et 
que toutes les pages virtuelles ajoutées par la suite (par malloc, par exemple) le
soient également. Ce qui veut dire que si lock est précisé on aura 7Gio de RAM
totalement consommés par le processus (e.g. toutes les pages seront présentes en RAM). 

Alors que si lock n'est pas précisé, la zone allouée par malloc n'a aucune garantie 
d'être totalement en mémoire physique a tout moment, il y aura des pages qui seront
en swap auquelles ont tentera d'accéder, ce qui générera un défaut de page majeur, 
puis le kernel les remettra en RAM, et on pourra écrire. 

3. A quoi sert l’entrée "install" du makefile ?

L'entrée install permet d'executer le fichier en tant qu'utilisateur effectif
root, afin de pouvoir changer la limite dure sur la mémoire via setrlimit. 
Puis, on changera le EUID au RUID de celui ayant executé le processus, (on perds donc les droits
si on en a moins que root), et par la suite on pourra se servir de autant de mémoire que l'on souhaite.

4. Décrivez le code et son fonctionement en détail.

(...)

5. Qu’est qu’un défaut de page mineur / majeur ?

Un défaut de page mineur arrive lorsqu'une page est en mémoire physique, mais n'est pas 
dans la table des pages du processus, il faut mettre a jour cette table afin de la faire
pointer sur la bonne page en mémoire physique.

Un défaut de page majeur arrive lorsque un processus souhaite accéder a une page qui n'est
pas en mémoire physique (par exemple elle est dans le swap car on a pas asser de place), 
a ce moment la le kernel vas devoir charger cette page en mémoire afin que le processus
puisse y accéder, si il n'y a pas asser de place, il mettra une page peut utilisée en swap,
puis il mettra a jour sa table des pages et le processus pourra y accéder.

6. Le programme effectue-t-il plus de défaut de page lorsque le paramètre "lock" est
passé ou sans paramètre ?

Avec lock en paramètre avec le flag MLC_FUTURE toute la mémoire virtuelle du processus sera
présente en mémoire RAM a tout moment, même lors d'augmentation de la pile ou du tas. Rien ne
pourras être mis en swap. Si on a asser de mémoire RAM disponible pour lock sur la machine, 
le programme la reservera entièrement, sinon il exite en manque de mémoire.

Sans lock en paramètre, malloc() vas allouer comme d'habitude un buffer en mémoire virtuelle,
mais les pages ne seront pas forcément toutes en mémoire a tout moment. La première fois qu'on
écrit dedans, si on a assez de RAM disponible, on ne fera aucune faute de page majeur, la première
fois. Par contre, après une fois qu'on n'utilise plus les pages, elles pourront être mises en swap
et en cas de nouvel accès, la ça créera des défauts de page majeurs. 

Si malloc alloue plus de mémoire virtuelle que de ram disponible, lors de la première écriture (e.g. initialisation a 0), 
le kernel vas mettre les pages précédentes en swap et remplir la mémoire au fur et a mesure, au deuxième passage lors
du strcpy, on sort les pages précédentes de la swap afin de copier des données dedans, ce qui des génère les défauts de page
majeurs.

Il peut toujours y en avoir d'autres sources / appels systèmes, c'est des
erreurs qui sont bénignes et que le kernel gère très bien.

7. Dans un cas général pourquoi utiliser la fonction mlock ?

Cela permet de réduire les temps de latence, car le processus n'a pas besoin d'attendre
que le noyau sorte une page du swap et la mette en mémoire physique, c'est un processus
qui prends du temps, surtout sur des disques dur plus vieux. Ce serait dans le cas
ou on a besoin d'une mémoire très rapide d'accès que l'on sait que lorsque l'on en a besoin
devra être accessible immédiatement.
Aussi si on veux que le programme s'execute parfaitement ou pas du tout.

*/