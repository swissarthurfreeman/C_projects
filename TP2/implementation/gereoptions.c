/************************************************
*  Implémentation de l'interface gereoptions    *
*  Auteur : Arthur Freeman, Date : 11/10/2020.  *
*************************************************/ 
#include <unistd.h> //pour getopt.
#include <stdlib.h> 
#include <stdio.h> //pour printf.
#include <string.h> //pour strlen, strncat.
#include "gereoptions.h"  //Interface à implémenter.

//Fonction qui parse et renvoie les options fournies par l'utilisateur.
struct arguments get_options(int argc, char *argv[]) {
    //Structure à renvoyer contenant les options.
    struct arguments argum; 
    int opt;
    //Par défaut, on suppose que -f et -t ne sont pas fournis.
    argum.f = 0;
    argum.t = 0;
    argum.digest = "sha1"; //default digest.

    //Du moment qu'il y a des options.
    while ((opt = getopt(argc, argv, "ft:")) != -1) {
        //switch opt se fait sur des caractères ! avec '' (comparaison entre int)
        switch(opt) {
            case 'f': //Si on a fourni le flag f
                argum.f = 1; //On set f à vrai.
                break;
            case 't': //Si on a fourni un digest.
                //On set le digest à l'argument suivant t. 
                argum.digest = optarg; //optarg est crée par getopt. 
                argum.t = 1;
                break;
            default: 
                //Sinon, c'est qu'on a reçu des options non gérées, donc on rappele l'usage.
                printf("Usage : [-f] [-t TYPE] STRING \n");
        }
    }
    
    //Si on a pas reçu de nom de fichier, il faut
    //mettre ce qui suit dans un buffer et le hasher.
    if (!argum.f) { 
        char space;
        space = ' ';
        int msg_length = 0;
        argum.message = malloc(sizeof(char)); //on alloue initialement, pour pouvoir utiliser realloc.
        //pour chaque mot depuis optind.
        for(int i = optind; i < argc; i++) {
            //on récupère la taille du mot.            
            size_t argSize = strlen(argv[i]);
            //on incrémente la taille du message 
            msg_length += argSize; 
            //on ralloue de cette quantité (realloc n'efface pas ce qui y était avant)
            //on étend la taille de la mémoire pointée par argum.message.
            //realloc renvoie un pointeur, qui peut être le même si on a réussi à allouer 
            //de façon continue. Realloc change la taille du bloc mémoire pointé 
            //par agrum.message par (msg_length)*sizeof(char) octets.
            argum.message = realloc(argum.message, (msg_length)*sizeof(char) );
            
            //on concatène le message en ajoutant un message.
            strncat(argum.message, argv[i], argSize);
            //on mets un espace entre chaque mot. (excepté le dernier)
            if (i < argc - 1) {
                strncat(argum.message, &space, sizeof(char));
            }
        }   
    }
    //Si optind est plus grand que le nombre d'arguments, c'est qu'il
    //y a un problème : on n'a pas donné assez d'arguments.
    if (optind >= argc) {
        printf("Error, expected more arguments. \n");
    }
    //on renvoie optind, pour pouvoir boucler sur les arguments dans main.c
    argum.optind = optind;
    return argum;
}