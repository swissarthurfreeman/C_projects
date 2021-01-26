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
#define MAX_PENDING 5

//pa configure la structure sockaddr_in en TCP avec port.
void pa( struct sockaddr_in *address , int port ) {
    //on initialise un buffer auquel stocker la structure sockaddr_in
    memset(address , 0, sizeof(address));
    //notre famille d'addresses est IPv4
    address ->sin_family = AF_INET;
    //on convertit en network byte order (big endian)
    //INADDR_ANY fait que toutes les interfaces locales
    //seront bind a ce socket.
    //s_addr est sur 4 octets.
    address ->sin_addr.s_addr = htonl( INADDR_ANY );
    //htons convertit port en big endian (port sur 2 octets).
    address ->sin_port = htons(port);
}

//fonction qui prends un numéro de port, crée un socket
//en TCP, fait un bind a une addresse quelconque et écoute dessus.
//on renvoie le descripteur de fichier du socket.
int ms( int port ) {
    struct sockaddr_in address;
    //on ouvre un socket en TCP avec une addresse IPv4 (PF_INET c'est AF_INET equivalent sur BSD)
    int sock = socket(PF_INET , SOCK_STREAM , 0);
    if( sock < 0 ) {
        die("Failed to create socket");
    }
    //on configure sockaddr_in
    pa( &address , port );

    //bind associe l'addresse au socket.
    if( bind( sock ,(struct sockaddr *) &address , sizeof(address)) < 0 ) {
        die("Failed to bind the server socket");
    }
    //listen marque le socket comme étant un socket d'écoute
    //e.g. on utilisera accept dessus.
    //la taille de la queue des connections en attente est maximale.
    if (listen(sock , MAX_PENDING ) < 0) {
        die("Failed to listen on server socket");
    }
    return sock;
}

void hc( int clientSock ) {
    time_t t;
    struct tm *now;
    //time renvoie le temps depuis l'epoch.
    time( &t );
    //on convertit en human readable, gmtime renvoie
    //un pointeur sur une structure tm. (qui contiends les secondes, minutes, heures, jours, mois, année...)
    now = gmtime( &t );
    //on envoie toute la structure au client.
    write( clientSock , now , sizeof(struct tm) );
    //on ferme le socket client.
    close( clientSock );
}

void run( int serverSock ) {

    while( 1 ) {
        struct sockaddr_in clientAddress ;

        unsigned int clientLength = sizeof( clientAddress );
        int clientSock ;
        printf( "Waiting for incoming connections\n");
        //clientSock est le descripteur de fichier permettant de communiquer avec le client.
        //serverSock est passif (marqué par listen()), clientAddress sera populée avec les info du client.
        //en fait cette interface permet d'avoir des sockets IPv6 aussi, car clientLength est fournie, donc
        //la taille peut changer (e.g. addresses IPv6 sur plus d'octets qu'IPv4)
        clientSock = accept(serverSock , (struct sockaddr *) &clientAddress , &clientLength );
        
        if( clientSock < 0 ) {
            die("Failed to accept client connection");
        }
        //convertit sin_addr (qui est en big endian) en représentation décimale pointée en little endian.
        printf( "Client connected: %s\n", inet_ntoa ( clientAddress.sin_addr));

        //on écrit le temps sur clientSock
        //hc écrit puis ferme la connection. 
        //accept bloque le programme jusqu'à l'apparition d'une connection !
        hc( clientSock );
    }
}

int main( int argc , char ** argv ) {
    int servSock;
    int port;

    if (argc != 2) {
        fprintf(stderr , "USAGE: %s <port>\n", argv [0]);
        exit( EXIT_FAILURE );
    }
    //atoi convertit un string en integer
    //e.g. le numéro de port.
    port = atoi(argv [1]);

    //on configure le socket.
    servSock = ms( port );

    printf( "Server running on port %d’\n", port );

    //on accepte des connections.
    run( servSock );
    close(servSock);

    return EXIT_SUCCESS ;
}

/* Questions :
1. Que font les deux programmes suivants ?


2. Décrivez en détail le code et son fonctionement.

(...)

3. Le code ’server.c’ comporte une boucle infinie. Comment pourrait-on l’interrompre
proprement ?

On fait avec une variable globale interrupted, lorsque l'on reçoit  sigint, on modifie
la valeur interrupted.

while(not Interrupted) {}

handler(sigint) {
    Interrupted = true
}

4. Le code du client devient problématique si la connection réseau est lente ou
mauvaise. Pourquoi ? Comment-y remédier ?

TCP est un protocole orienté connection, avec les acks de confirmation contrôle de
flux c'est un protocole qui est plus lourd mais plus robuste. Alors que datagrammes:
on jette tout dans la nature et le réseau se demmerde, datagrammes est mieux adapté ici, 
car on ne write qu'une seule fois et il est beaucoup plus léger. On pourrait mettre en
datagrammes si la date récupérée n'est pas essentielle afin de faire tourner le système,
des erreurs pourraient avoir lieu vu que c'est moins robuste.

*/