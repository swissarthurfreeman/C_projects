#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#define NB_LIGNES 5
#define NB_COLS 5
#define MEM_NAME "/plateau"

void exit_err (const char* str) {
    perror(str);
    exit( EXIT_FAILURE );
}

char val_tab(const char*plateau , int x , int y) {
    //0 c'est NULL en ASCII mais en entiers c'est 0 
    //1 c'est SOH mais en entiers c'est 1 donc tout vas bien ^^
    //on a pas assigné '0' et '1' mais 0 et 1.
    return plateau[NB_COLS*x + y];
}

/* Cette fonction affiche le plateau toujours au meme
endroit sur l’ecran , vous n’aurez pas de question sur
cette fonction */
void affiche_plateau (const char* plateau) {
    int i,j;
    for(i=0;i < NB_LIGNES ; i++)
       for(j=0;j < NB_COLS; j++)
            printf("\033[%d;%dH%d", i+1, j+1, val_tab(plateau , i, j));
}

int compte(char* plateau , int x, int y) {
    /* On regarde dans toutes les cellules voisines.
    avec x+i et y+j
    ------------------------
    |       |       |       |
    |       |       |       |
    |-----------------------|
    |       |       |       |
    |       | x, y  |       |
    |-----------------------|
    |       |       |       |
    |       |       |       |
    ------------------------
    */
    int i, j, cpt = 0;
    for(i=-1; i < 2 ; i++)
        for(j=-1; j < 2; j++)
            if( !((i == 0) 
                && (j == 0)) //on ne regarde pas la cellule elle même. 
                && (x+i >= 0) && (y+j >= 0)   //on exclu les cas en dehors du cadre.
                && (x+i < NB_LIGNES ) && (y+i < NB_COLS))
                cpt += val_tab(plateau , x+i, y+j); //on incrémente le nb de cellules vivantes.

    return cpt;
}

void cellule(char* plateau , int x, int y) {

    //ces deux lignes sont une heuristique afin de déterminer si vivante ou pas initialement je crois...
    char *cell = ( plateau + NB_COLS * x + y );
    // % -> modulo (reste de la division)
    *cell = getpid () % 2;
    while(1) {
        //compte renvoie le nombre de cellules vivantes
        //autour de la cellule (x,y)
        int cpt = compte(plateau , x, y);
        if(*cell > 0) {
            //cellule vivante si elle possède
            //deux ou trois voisines vivantes.
            //e.g. morte si elle possède moins de deux
            //ou plus de 3 voisines vivantes.
            if( (cpt < 2) || (cpt > 3) )
                *cell = 0;
        }
        else if(cpt == 3) //si elle a 3 voisines, elle vit
            *cell = 1; //si elle était déjà vivante, ça change rien.

        sched_yield ();
    }

    exit( EXIT_SUCCESS );
}

int main () {
    //le plateau est un string.
    char *plateau;

    int fd_mem;
    
    size_t mem_size = NB_LIGNES * NB_COLS * sizeof(char);
    
    //on ouvre un objet mémoire partagée
    //ce fichie rsera visible dans /dev/shm/MEM_NAME
    //mais il faudra encore le mettre dans l'espace virtuel du processus
    //avec mmap.
    //les permissions seront rw par rapport a l'utilisateur
    fd_mem = shm_open(MEM_NAME , O_RDWR | O_CREAT | O_EXCL , 0600);
    
    if (fd_mem == -1)
        exit_err("main , shm_open");

    //on devrait faire ça après le mmap...?
    shm_unlink ( MEM_NAME );
    
    //on configure la taille du fichier, mem_size est la taille du plateau
    //en bytes.
    if( ftruncate (fd_mem , mem_size) == -1)
        exit_err("main , ftruncate");

    /*
    On mets le plateau /dev/shm/plateau en mémoire virtuelle du processus
    LES ENFANTS HÉRITENT TOUS CETTE MÉMOIRE PARTAGÉE !
    Fork crée une copie intégrale du segment de mémoire du parent chez l'enfant !
    cela inclut le segment de mémoire partagée !
    l'addresse NULL veut dire que le noyau choisit l'addresse
    mmap crée ce mapping dans l'espace d'adressage virtuel du processus.
    MAP_SHARED fait en sorte que d'autre processus ayant mappé la même région puisse voir
    les modifications.
    PROT_READ | PROT_WRITE veut dire que les pages peuvent être lues ou écrites.
    mmap renvoie un pointeur vers la mémoire partagée
    */
    plateau = (char*) mmap(NULL , mem_size , PROT_READ | PROT_WRITE , MAP_SHARED , fd_mem , 0);
    
    if (plateau == MAP_FAILED )
        exit_err("main , mmap");
    
    close(fd_mem);

    //Le code commente ci-dessous est en lien avec une des question cidessus
    /*
    struct sched_param sp;
    sp.sched_priority = 1;
    sched_setscheduler(getpid(), SCHED_FIFO , &sp);
    */
    int i, j;
    /*
    ici on donne l'administration de la cellule a chaque enfant,
    chaque enfant hérite de la mémoire partagée, et donc modifiera 
    plateau, qui est un pointeur vers celle ci, et c'est un string représentant
    le plateau.
    */
    for(i=0; i < NB_LIGNES ; i++) {
        for(j=0; j < NB_COLS; j++) {
            //sur chaque case on forke.
            pid_t res = fork ();
            if(res == -1)
                exit_err("main , fork");
            else if(res == 0)
                cellule(plateau , i, j); //on administre la cellule
        }
    }

    while(1)
        affiche_plateau (plateau);

    if(munmap(plateau , mem_size) == -1)
        exit_err("main , munmap");
    return 0;
}

/*
Questions:

1. Combien de processus ce programme génère-t-il ? Décrire le rôle et les actions
de chaque processus.

Ce programme génère NB_LIGNES * NB_COLONNES processus, ou chaque processus
administre une cellule. Un enfant ne reforke pas de nouveau, car l'appel 
cellule contient un while dont on ne sort jamais.

2. Dans le cas ou le processus principal (parent) meurt qu’arrive-t-il aux enfants de
ce processus ?

ça dépends de comment le parent meurt, si on effectue un Ctrl+C dans le shell
et que le programme est en tache principale, Ctr+C enverra SIGINT au pgid (process group id)
du processus en tâche principale, e.g. au parent et tous ses enfants, donc les enfants 
mourront aussi. 

Si le parent est tué par un signal indépendamment, par exemple par : kill -s SIGINT pid_parent
les enfants continueront a tourner sans problème. Si a ce moment la on veux les tuer, Ctrl+C
n'ayant plus de tâche en tâche principale, il faudra envoyer un signal de terminaison au groupe
des processus enfants, qui peut se récupérer avec

ps -ax -O "pgid"

et on le tue en envoyant kill -s SIGINT -GROUP_PROCESS_ID

3. quelle est l’utilité de la fonction sched_yield (ligne 59) dans ce programme.


4. Y-a-t-il des conflits dans les ressources utilisées par les différents processus ?
Expliquez.


5. Imaginons que ce programme tourne sur un proce
*/