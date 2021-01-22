#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Un fifo a une extrémité on write et a une extrémité on read, les deux
//processus peuvent le faire. (A ecrit, B lit, B écrit A lit...)

/*Ce code marche parfaitement en faisant les test...*/
int handle(char *name) {
    ssize_t nread, nwrit;
    int fifo = -1;
    long buffer = 0; //integer sur 32 bits.

    //on crée un named pipe avec name comme nom et permissions 0600 (rw user)
    //les permissions du fichier sont mode & ~umask
    int result = mkfifo( name , 0600 );

    if( result < 0 ) {
        perror( "Cannot create the fifo" );
        exit( EXIT_FAILURE );
    }

    //on ouvre le named pipe
    fifo = open( name , O_RDWR );
    if( fifo < 0 ) {
        perror( "Cannot open the fifo" );
        exit( EXIT_FAILURE );
    }

    //read est bloquant jusqu'à ce que l'autre processus
    //écrive dans le pipe.
    //read renvoie le nombre de bytes lu,
    //si le nb est plus petit que sizeof(buffer) ça veut
    //dire qu'on a été interrompu, il faudrait vérifier
    //que nread = sizeof(buffer)
    //4 octets de taille
    nread = read(fifo , &buffer , sizeof(buffer));
    if ( nread < 0 ) {
        perror( "Reception failure" );
        exit( EXIT_FAILURE );
    }

    //on gère pas le cas d'overflow.
    buffer *= 2;


    nwrit = write( fifo , &buffer , sizeof(buffer));
    if ( nwrit < 0 ) {
        perror( "Transmission failure" );
        exit( EXIT_FAILURE );
    }
    //on ne remove pas le mkfifo après l'avoir crée, c'est un problème.
}

/*
1. Quel est l’intention de l’extrait de code ci-dessous ?

D'avoir un autre processus qui envoie des valeurs et qui reçoit le
résultat multiplié par 2.

2. Pourquoi est-ce que cette approche est vouée à l’échec ?

Cette approche marche parfaitement.

3. Comment y remédier ?

???

*/