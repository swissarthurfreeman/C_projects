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
    lseek( fd , 0, SEEK_SET );
    for( i=0; i < ACC_SIZE; i++ ) {
        read( fd , &a, SIZE );
        printf( "%d: %d\n", i, a );
    }
}

int get( int fd , int acc ) {
    int off = acc * SIZE;
    int a = -1;
    lseek( fd , off , SEEK_SET );
    read( fd , &a, SIZE );
    return a;
}

void set( int fd , int acc , int total ) {
    int off = acc * SIZE;
    lseek( fd , off , SEEK_SET );
    write( fd , &total , SIZE );
}

void acquire( int fd , int acc ) {
    struct flock fl;
    //verrou du fichier en écriture
    fl.l_type = F_WRLCK;
    //depuis le début du fichier
    fl.l_whence = SEEK_SET;

    //offset de départ on bloque le compte
    //acc pendant la transaction
    fl.l_start = acc;

    //nombre de bytes a vérouiller.
    //c'est pas plutôt un int qu'il faut vérouiller ? e.g. sizeof(int) ? 
    fl.l_len = 1;

    //on place le verrou on attends si y'en avait
    //déjà un.
    fcntl(fd , F_SETLKW , &fl);
}

void transfer ( int fd , int from , int to , int amount ) {
    int fromTotal , toTotal;
    //acquire place un verrou d'écriture
    //sur la valeur du compte N°from.
    acquire( fd , from );
    
    fromTotal = get( fd , from );
    if( fromTotal < amount ) {
    printf("Not enough money in account: %d\n", from );
    return;
    }
    acquire( fd , to );
    toTotal = get( fd , to );
    set( fd , from , fromTotal -amount );
    set( fd , to , toTotal+amount );
}

int main( int argc , char ** argv ) {
    char* dataFile = argv [1];
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
    close(fd);
    return EXIT_SUCCESS ;
}

/*
1. Que font les programmes suivant ?
2. Décrivez en détail le code et son fonctionement.
3. Ce code peut poser problème si plusieurs processus indépendants essayent
d’accéder simultanéments au mêmes comptes. Expliquez ?
4. Comment résoudre ce problème ?
*/