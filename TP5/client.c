#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define PORT 65104

#define TOO_LOW 0
#define TOO_HIGH 1
#define WIN 2
#define LOSE 3

int get_number() {
    int n;
    scanf("%d", &n);
    return n;
}

int main(int argc, char argv[]) {
    int client = socket(AF_INET, SOCK_STREAM, 0);
    if(client < 0) {
        //traiter erreur
    }
    struct sockaddr_in address;
    //inet_pton convertit l'adresse fournie en structure in_addr
    //et la copie vers address. (lorsque AF_INET est mis en paramètre).
    int res = inet_pton( AF_INET, "127.0.0.1", &(address.sin_addr) );
    if(res == 0) {
        printf("Invalid IP address\n");
        exit(-1);
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT); 

    connect(client, (struct sockaddr *) &address, sizeof(address));

    //read one byte on client socket.
    unsigned char min, max; 
    printf("Reading \n");
    read(client, &min, 1);
    read(client, &max, 1);
    printf("[min=%d, max=%d] \n", min, max);
    
    while (1) {
        //on envoie ce que l'utilisateur a entré.
        int input = get_number();
        
        //printf("input is = %d\n", input);
        unsigned char casted;
        casted = (unsigned char) input;
        write(client, NULL, 1);
        write(client, &casted, 1);

        //on récupère ce que le serveur aura répondu.
        unsigned char cmd;
        read(client, &cmd, 1);
        read(client, &res, 1);
        if (cmd == WIN) {
            printf("C'est le bon nombre ! \n");
            close(client);
            break;
        } else {
            switch (cmd) {
                case LOSE:
                    printf("Trop d'essais, essayez la recherche dichotomique...\n");
                    exit(1);
                case TOO_HIGH:
                    printf("[cmd=TOO_HIGH, valeur=%d] \n", res);
                    break;
                case TOO_LOW:
                    printf("[cmd=TOO_LOW, valeur=%d] \n", res);
                    break;
            }
        }
    }
    return 0;
}