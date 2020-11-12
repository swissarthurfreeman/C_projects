#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

//J'utilise des entiers non signés pour plus de facilité.
#define TOO_LOW 0
#define TOO_HIGH 1
#define WIN 2
#define LOSE 3
#define ESSAIS 10

#define PORT 65104

int main(int argc, char argv[]) {
    
    //on crée un socket en IPv4, en mot connexion oriented
    //stream de protocole TCP.
    int serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_fd <= -1) {
        //en cas d'erreur de création de socket.
        printf("Socket Creation Error");
        exit(-1);
    }
    
    struct sockaddr_in addresse;
    addresse.sin_family = AF_INET;
    //conversion en big endian (représentation network) via htons (htons pour unsigned short integer)
    addresse.sin_port = htons(PORT);
    //idem ici, excepté que ce n'est pas un short int, mais un int (32 bits), donc htonl.
    //INADDR_ANY écoutera sur toutes les interfaces réseau de la machine, du moment
    //que le port spécifié est le bon. Le client y accédera avec l'addresse localhost et le port.
    addresse.sin_addr.s_addr = htonl(INADDR_ANY);

    //on attribue une adresse IP au socket.
    int res = bind(serv_fd, (struct sockaddr *) &addresse, sizeof(addresse)); 
    
    if (res != 0) {
        //En cas d'erreur de binding.
        printf("Oh shit, is a WTF binding error.\n");
        printf("%s\n", strerror(errno));
        exit(-1);
    }

    printf("Server listening on port %i\n", PORT);

    //on configure le serveur comme étant un serveur d'écoute passif.
    int queue = 5;
    int c = listen(serv_fd, queue);
    if ( c < 0) { printf("Listening call error \n"); }
    
    //boucle d'écoute.
    //int clients[queue];

    //FAIRE CECI POUR CHAQUE CLIENT
    //int clients[queue]
    //nombre entre 23 et 148.
    unsigned char number = random() % 126 + 23;
    printf("number is : %d\n", number);
    unsigned char min = 23;
    unsigned char max = 148;

    //on initialise l'addresse du client, elle sera configurée
    //par l'appel à accept.
    struct sockaddr_in addresse_client;
    unsigned int longueur_client = sizeof(addresse_client);
    //on récupère le descripteur de fichier du socket du client, on caste comme d'habitude
    //avec la structure sockaddr *, car ces interfaces gèrent IPv4 et IPv6.   
    //acceptation de la première connection.
    int socket_client = accept(serv_fd, (struct sockaddr *) &addresse_client, &longueur_client);
    write(socket_client, &min, 1);
    write(socket_client, &max, 1);

    printf("Client IP address is %s, fd : %i \n", inet_ntoa(addresse_client.sin_addr), socket_client);
    int nb_essais = 0;
    unsigned char cmd;
    while(1) {
        if (nb_essais > 10) {
            cmd = LOSE;
            write(socket_client, &cmd, 1);
            write(socket_client, &number, 1);
        }
        //on lit l'essai de l'utilisateur.
        unsigned char res;
        read(socket_client, NULL, 1);
        read(socket_client, &res, 1);

        printf("user_guess = %d\n", res);

        //si la réponse de l'utilisateur est le bon nombre.
        if (res == number) {
            printf("Client won ! \n");
            cmd = WIN; //WIN
            write(socket_client, &cmd, 1); //on lui dit qu'il a gagné.
            write(socket_client, &number, 1);
            close(socket_client); //on ferme.
            close(serv_fd);
            break;
        } else {
            nb_essais = nb_essais + 1;
            if(res > number) {
                cmd = TOO_HIGH;
                write(socket_client, &cmd, 1);
                write(socket_client, &number, 1);
            } else {
                cmd = TOO_LOW;
                write(socket_client, &cmd, 1);
                write(socket_client, &number, 1);
            }
        }
    }
    return 0;
}