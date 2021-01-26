#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void OnError(char *str) {
    perror(str);
    exit( EXIT_FAILURE );
}

//cette fonction vas parcourir tout le fichier en mémoire partagée
//et remplacer le charactère replaceChar par char dans celui ci.
int processFile (char* fileContent , off_t fileSize , char replaceChar , char byChar) {
    int i;

    printf("Processing char %c to be replaced by %c\n", replaceChar , byChar);
    
    for(i = 0 ; i < fileSize ; i++)
        if( fileContent [i] == replaceChar )
            fileContent [i] = byChar;

    return 0;
}

int main(int argc , char* argv []) {
    int fd , iArg;

    pid_t fork_res;
    
    struct stat fileStat;
    char* fileContent ;

    //il faut 4 arguments minmum, et un nombre pair si plus grand.
    if(( argc % 2) || (argc < 4)) {
        printf("Wrong number of arguments: %d\n\n", argc);
        printf("Usage:\n\t sharedmem filename replaceChar byChar [replaceChar2 byChar2 [replaceChar3 byChar3 [...]]]\n\n");
        exit( EXIT_SUCCESS );
    }

    //utilise pas shm open mais le fichier fourni, on ne le crée pas si il n'existe pas.
    if((fd = open(argv [1], O_RDWR)) == -1)
        OnError("Cannot open file");

    //fstat fonctionne comme stat mais sur un file descriptor.
    if( fstat(fd , &fileStat ) == -1)
        OnError("fstat");

    //on ouvre une mémoire partagée en lecture et écriture dans laquelle ont mets le fichier (fd).
    //cette mémoire partagée sera dans le segment d'espace virtuel du processus appelant.
    fileContent = (char*) mmap(NULL , (size_t) fileStat.st_size , PROT_READ | PROT_WRITE , MAP_SHARED , fd , 0);
    
    if( fileContent == MAP_FAILED )
        OnError("mmap");

    //on ferme le fichier. (cela n'invalide pas le mmap)
    close(fd);

    //depuis le deuxième argument jusqu'au dernier par paires de 2
    for(iArg = 2 ; iArg < argc ; iArg += 2) {
        //on crée un enfant (Il copie le mapping dans sa mémoire virtuelle,
        //mais en RAM ce sont les même pages partagées qu'il accédera)
        fork_res = fork ();
        
        if(fork_res == -1)
            OnError("Fork");
        //dans l'enfant, on processFile, fileContent est un pointeur sur la mémoire partagée.
        //fileStat.st_size est la taille du fichier.
        else if(fork_res == 0) //on déréférence afin de récupérer les charactères.
            return processFile ( fileContent , fileStat.st_size , *argv[iArg], *argv[iArg + 1]);
    }
    //on attends que chaque enfant meure.
    for(iArg =2; iArg <argc;iArg +=2)
        wait(NULL);

    //on enlève la mémoire partagée de la mémoire, MAP_SHARED fera que le contenu
    //du fichier aura bien été modifié.
    if(munmap(fileContent , fileStat.st_size) == -1)
        OnError("munmap");

    return 0;
}

/*
1. Que fait le programme suivant ?

Ce programme prends un fichier fournit par l'utilisateur
et une liste de paires de valeurs (carToReplace, replaceBy) a remplacer
dans le fichier, que l'on mappe en mémoire et on délégue un couple 
a chaque enfant du processus afin de ce faire plus vite. 
En modifiant le fichier en mémoire, les modifications seront faites
au fichier sur le disque avec MAP_SHARED.
MAP_SHARED fera que les modifications seront écrites au fichier.  

https://www.gnu.org/software/libc/manual/html_node/Memory_002dmapped-I_002fO.html

2. Décrivez le code et son fonctionement en détail.

(...)

3. A l’aide d’un schema expliquez ce que fait l’appel à la fonction mmap.

https://www.gnu.org/software/libc/manual/html_node/Memory_002dmapped-I_002fO.html

(schéma dans dossier) 

4. Que deviens la mémoire partagée lors de l’appel aux fonctions fork ? Expliquez
par un schema.

Les enfants héritent de la mémoire partagée et elle est conservée.

Les fork fonctionnent en copy on write, l'idée c'est on ne copie pas tout en mémoire
physique entre un enfant et un parent, si on ne modifie pas certaines pages, e.g.
certaines librairies partagées, fork() suit ce principe si MAP_PRIVATE est spécifié,
mais avec MAP_SHARED, chaque enfant modifiera la mémoire partagée au même endroit dans
la RAM. (et donc le fichier)

Fork effectue une copie de l'espace mémoire virtuel du processus parent, dont
le segment partagé, qui pointera sur le même endroit en mémoire physique, car
il a été ouvert par mmap sur un même fd par chaque enfant (dans le parent avant le fork).
Et avec le flag MAP_SHARED, on effectuera pas de copy on write.


5. Ce programme peut ne pas fonctionner dans le cas de l’appel suivant:
sharedmem monfichier a b b d pourquoi ? 

C'est un comportement non déterministe, on a les processus (a, b) et (b, d)
il pourrait se passer que le processus (a, b) remplace le premier a et pendant
l'écriture du remplacement, le processus (b, d) lit a et passe a la suite, e.g.
(a, b) remplacera a par b, mais (b, d) ne remplacera pas b par d. 

Ces modifications ne sont pas séquentielles mais parallèles, c'est l'horreur,
on a aucune garantie de remplacer tout b par d, car si le processus A remplace
un a par b, mais que le processus B est plus loin dans le fichier, le b ne sera
pas remplacé par d. Il faudrait changer l'ordonnancement de processus et mettre
en file, mais autant le faire séquentiellement. 

6. Quel serait l’effet d’utiliser MAP_PRIVATE au lieu de MAP_SHARED dans la
fonction mmap ? Cela permet-il de resoudre le problème ci-dessus ?

MAP_PRIVATE crée un mapping privé les modifications sur celui-ci ne seront pas visibles
chez d'autres processus et ne seront pas réfléchies dans le fichier sur le disque.
Si un processus modifie le contenu de la mémoire, il créera sa propre copie des pages, 
et le fichier ne sera pas modifié.

Il faut choisir qui modifie en premier, et MAP_PRIVATE ne résoud pas ce problème.

e.g. cela ne permet pas de résoudre le problème, on utilise cela plutôt si on veux
faire un read sur ce fichier sans qu'un autre processus puisse le modifier sous nos yeux.

7. Est-ce toujours un avantage d’avoir plusieurs processus qui travaillent 
parallèlement sur une mémoire partagée plutôt que un processus travaillant séquentiellement ? Pourquoi ?

Pas toujours, c'est surtout dans les cas ou ils font des tâches qui ne peuvent pas empieter l'une sur 
l'autre, si il y a une notion d'ordre, e.g. la tâche B doit être faite APRÈS la tâche A, on ne peux pas 
utiliser une memoire partagée avec plusieurs processus afin de ce faire, autant le faire séquentiellement 
comme d'habitude ou bien il faudrait mettre des locks de mémoire...

*/