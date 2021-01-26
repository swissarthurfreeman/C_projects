#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "common.h"

void pa( struct sockaddr_in *address , const char *host , int port ) {
    memset(address , 0, sizeof(struct sockaddr_in ));
    address -> sin_family = AF_INET;
    //inet_pton convertit l'addresse en une addresse big endian et la copie dans dst.
    inet_pton ( AF_INET , (char*) address , &( address -> sin_addr) );
    //idem avec le port.
    address ->sin_port = htons(port);
}

int ms( const char *host , int port ) {
    struct sockaddr_in address;
    //on crée un socket client en TCP.
    int sock = socket(PF_INET , SOCK_STREAM , 0);
    if( sock < 0 ) {
        die("Failed to create socket");
    }
    //on configure la structure addresse avec host et port.
    pa( &address , host , port );
    //on se connecte au host, on renvoie le socket.
    if( connect(sock , (struct sockaddr *) &address , sizeof(struct sockaddr_in )) < 0) {
        die("Failed to connect with server");
    }
    return sock;
}

int get( int socket , struct tm *answer ) {
    //le read sera bloquant jusqu'à avoir reçu ce qu'il faut.
    int r = read( socket , answer , sizeof(struct tm) );
    return r;
}

void display( struct tm *t ) {
    printf( "%d/%d/%d - %d:%d:%d\n", t->tm_mday , (t->tm_mon +1) , (t->tm_year +1900) , t->tm_hour , t->tm_min , t->tm_sec );
}

int main(int argc , char *argv []) {
    int sock;
    char *host;
    int port;
    struct tm answer;

    if (argc != 3) {
        fprintf(stderr , "USAGE: %s <host> <port>\n", argv [0]);
        exit( EXIT_FAILURE );
    }

    host = argv [1];
    port = atoi(argv [2]);
    //on configure un socket client
    sock = ms( host , port );

    if( get(sock , &answer) < 0 ) {
        die( "Reception error." );
    }

    close(sock);

    //on affiche le résultat.
    display (&answer);

    exit( EXIT_SUCCESS );
}

