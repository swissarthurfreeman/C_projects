#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

//J'utilise des entiers non signés 
//pour plus de facilité.
//On envoie des unsigned char (codés sur 8 bits)
#define TOO_LOW 0
#define TOO_HIGH 1
#define WIN 2
#define LOSE 3
#define ESSAIS 10

int main(int argc, char * argv[]) {
    int PORT = strtol(argv[1], NULL, 10);
    //on ne veux pas que l'utilisateur fournissent n'importe quoi en entrée.
    if (PORT > 65535 || PORT < 1024) { 
        printf("Error, port supplied isn't adequate\n(must be between 1024 and 65'535) \n");
        exit(-1);
    }

    //on crée un socket en IPv4, en mode connexion oriented
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
        printf("Binding Error\n");
        //Souvent c'est que l'adresse fournie est déjà utilisée.
        printf("%s\n", strerror(errno));
        exit(-1);
    }

    printf("Server listening on port %i \n", PORT);

    //on configure le serveur comme étant un serveur d'écoute passif.
    int queue = 5;
    int c = listen(serv_fd, queue);
    if ( c < 0) { printf("Listening call error \n"); }
    
    //nombre entre 23 et 148. même intervalle pour tout client.
    unsigned char min = 23; 
    unsigned char max = 148;
    accept:printf(" ");
    //on initialise l'addresse du client, elle sera configurée
    //par l'appel à accept.
    struct sockaddr_in addresse_client;
    unsigned int longueur_client = sizeof(addresse_client);
    //on récupère le descripteur de fichier du socket du client, on caste comme d'habitude
    //avec la structure sockaddr *, car ces interfaces gèrent IPv4 et IPv6.   
    //acceptation de la première connection.
    int socket_client = accept(serv_fd, (struct sockaddr *) &addresse_client, &longueur_client);
    
    printf("Client %i address is %s \n", socket_client, inet_ntoa(addresse_client.sin_addr));
    pid_t pid = fork(); //a chaque accept on fork le processus.
    //si on est dans le parent (pid > 0)
    if (pid > 0) {
        //car on administre ce socket dans le fork.
        close(socket_client);  //on aura toujours les mêmes numéros...
        goto accept; //on reaccepte une connection.
    //si le pid est nul c'est l'enfant
    } else if (pid == 0) {
        //on lui recrée un enfant, afin d'éviter les zombies.
        pid_t pid_p = fork();  
        if (pid_p == 0) {
            //il faut initialiser le générateur de nbs premiers. (seed)
            srand(time(NULL));
            //on génère avec rand, je ne lit pas dans /dev/urandom car je suis sur
            //WSL et le fichier n'est pas fonctionnel. Rand fonctionne parfaitement.
            unsigned char number = rand() % 126 + min;
            printf("La valeur %d est choisie pour le client %d \n", number, socket_client);
            
            //On envoie la borne inférieure et supérieure au client.
            //on écrit sur le fd du socket, on envoie 1 octet (1 char)
            write(socket_client, &min, 1); 
            write(socket_client, &max, 1);
            
            int nb_essais = 0;
            unsigned char cmd;
            while(1) {
                //si on a fait trop d'essais
                if (nb_essais > 10) {
                    cmd = LOSE; //on a perdu
                    write(socket_client, &cmd, 1); //on notifie le client
                    write(socket_client, &number, 1);
                    exit(EXIT_SUCCESS); //et on ferme le serveur (cette instance)
                }
                //on lit l'essai de l'utilisateur.
                unsigned char res;
                read(socket_client, NULL, 1); //le premier octet est ignoré
                read(socket_client, &res, 1); //le deuxième contient la réponse

                printf("Client %d propose %d \n", socket_client, res);

                //si la réponse de l'utilisateur est le bon nombre.
                if (res == number) {
                    printf("Client %d a gagne ! \n", socket_client);
                    cmd = WIN; //WIN
                    write(socket_client, &cmd, 1); //on lui dit qu'il a gagné.
                    write(socket_client, &number, 1);
                    exit(EXIT_SUCCESS);
                } else {
                    //sinon, on incrémente le nombre d'essais
                    nb_essais = nb_essais + 1;
                    if(res > number) { //si l'essai est trop grand
                        cmd = TOO_HIGH; //on lui dit
                        write(socket_client, &cmd, 1);
                        write(socket_client, &number, 1);
                    } else { //si c'est trop petit
                        cmd = TOO_LOW; //idem
                        write(socket_client, &cmd, 1);
                        write(socket_client, &number, 1);
                    }
                }
            }
        }
        //on ne veux pas savoir ce qui est arrivé à l'enfant (d'ou NULL)
        waitpid(-1, NULL, 0); 
    }
    return 0;
}