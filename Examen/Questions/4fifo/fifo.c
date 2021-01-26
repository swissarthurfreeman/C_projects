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

    //on ouvre le named pipe faut pas ouvrir en RDWR !
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

C'est le code d'un worker qui fait une opération sur un entier qu'il
multiplie par 2 et qui renvoie a celui qui écrit dans le named pipe.

D'avoir un autre processus qui envoie des valeurs et qui reçoit le
résultat multiplié par 2.

2. Pourquoi est-ce que cette approche est vouée à l’échec ?

Si il y a plusieurs clients, ça peut mener a des désastres.
Si ont a plusieurs clients, si client1 envoie val1 a worker,
et se mets a read en attendant resultat1, mais que pendant que
worker calcule, un client2 écrit valeur2 a calculer, valeur2
sera dans le fifo, mais client1 attendant une valeur a lire il 
la lira, et au lieu d'avoir resultat1 il aura valeur2. 

man 7 fifo

3. Comment y remédier ?

On utilise des sockets, avec un socket par utilisateur et un canal
d'échange incontaminable par d'autres processus.

*/