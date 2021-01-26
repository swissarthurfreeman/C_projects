#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define ACC_SIZE 5
#define SIZE sizeof(int)

//fonction qui affiche tous les comptes en format
//N°compte : montant
void display( int fd ) {
    int i, a;
    //on se replace au début
    lseek( fd , 0, SEEK_SET );
    //ACC_SIZE = 5 on a 5 comptes de 0 a 4
    for( i=0; i < ACC_SIZE; i++ ) {
        //on lit un int a chaque fois
        read( fd , &a, SIZE );
        //on affiche numéro de compte et montant.
        printf( "%d: %d\n", i, a );
    }
}


int get( int fd , int acc ) {
    int off = acc * SIZE;
    int a = -1;
    //on se place au début du compte
    lseek( fd , off , SEEK_SET );
    //on récupère la valeur
    read( fd , &a, SIZE );
    return a;
}

void set( int fd , int acc , int total ) {
    int off = acc * SIZE;
    //on se place au début du compte
    lseek( fd , off , SEEK_SET );
    //on écrit un integer
    write( fd , &total , SIZE );
}

//acquire se place au bon endroit dans le
//fichier et verouille cette valeur.
void acquire( int fd , int acc ) {
    struct flock fl;
    //verrou du fichier en écriture
    fl.l_type = F_WRLCK;
    //depuis le début du fichier
    fl.l_whence = SEEK_SET;

    //offset de départ on bloque le compte
    //acc pendant la transaction
    //ça devrait être acc * sizeof(int) ?? 
    fl.l_start = acc;

    //nombre de bytes a vérouiller.
    //c'est pas plutôt un int qu'il faut vérouiller ? e.g. sizeof(int) ? 
    fl.l_len = 1;

    //on place le verrou on attends si y'en avait
    //déjà un.
    fcntl(fd , F_SETLKW , &fl);
}

//fonction qui effectue un transfert entre deux comptes
void transfer ( int fd , int from , int to , int amount ) {
    int fromTotal , toTotal;
    //acquire place un verrou d'écriture
    //sur la valeur du compte N°from.
    acquire( fd , from );

    //on récupère la valeur du compte
    fromTotal = get( fd , from );
    //si on a assez
    if( fromTotal < amount ) {
        printf("Not enough money in account: %d\n", from );
        return;
    }
    //on vérouille le compte vers qui on transfert
    acquire( fd , to );

    //on récupère la valeur dans l'autre compte qu'on aura vérouillé
    toTotal = get( fd , to );
    
    //on fait l'échange
    set( fd , from , fromTotal - amount ); //celui qui paie
    set( fd , to , toTotal + amount ); //celui qui se fait payer
}

int main( int argc , char ** argv ) {
    //on spécifie le nom du fichier en paramètre.
    //(avant ça il faudra avoir initialisé le fichier)
    char* dataFile = argv[1];
    //on devrait pas avoir de flag O_CREAT ici...
    int fd = open( dataFile , O_RDWR | O_CREAT , 0600 );
    printf( "=== AVANT =======================\n" );
    //on affiche l'état du compte
    display( fd );

    //on effectue des transferts entre comptes.
    transfer( fd , 0, 1, 25 );
    transfer( fd , 3, 2, 310 );
    transfer( fd , 4, 1, 10 );

    printf( "\n=== APRES =======================\n" );
    display( fd );
    //close relâche les verrous du processus.
    close(fd);
    return EXIT_SUCCESS ;
}

/*
1. Que font les programmes suivant ?

Le programme initialize.c crée un fichier dans lequel on note les différents
comptes avec différents montants présents de façon contigue. Le fichier a la tête
suivante :
acc  0       1        2       3           4 
------------------------------------------------
| int 100 | int 20 | int 50 | int 1000 | int 5 |
------------------------------------------------
si acc = 2         ^~~~~~~~~

2. Décrivez en détail le code et son fonctionement.

(...)

3. Ce code peut poser problème si plusieurs processus indépendants essayent
d’accéder simultanéments au mêmes comptes. Expliquez ?

https://www.gnu.org/software/libc/manual/html_node/File-Locks.html

Les appels systèmes read write ne vérifient pas la précense ou non de verrous.
Les verrous posés via fcntl sont des ADVISORY LOCKS, TOUT LE MONDE DOIT COOPÉRER.
Il faut que les utilisateurs ayant droits se mettent d'accord afin de vérifier
qu'il y ait un verrou présent ou pas. Sinon c'est le chaos, surtout si deux 
transferts sont effectués en même temps sur un même compte, on peut avoir des cas
spéciaux comme : 

Process A, B
A récupère la valeur dans acc, B aussi, la valeur a ce moment est de 10'000
A fait un transfert de 6000 vers acc2, B aussi, on aura transféré 12'000 au 
total, et pourtant la valeur du compte acc finale sera 4000, e.g. on a crée
de l'argent. C'est ce qui se passe si ils ne placent pas de verrou exclusif
lors de la manipulation des comptes.

Bref, on doit se mettre d'accord !

4. Comment résoudre ce problème ?

Afin de résoudre ce problème les processus DOIVENT ABSOLUMENT vérifier
la précense ou non de verrous. Ou bien on peut faire du MANDATORY LOCKING mais
il faut spécialement l'activer sur le filesystem et c'est pas standard.

*/