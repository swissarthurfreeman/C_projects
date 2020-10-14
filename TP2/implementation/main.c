/************************************************
*  Fichier principal du programme de hashing.   *
*  Auteur : Arthur Freeman, Date : 11/10/2020.  *
*************************************************/ 
#include <stdio.h> //Pour printf.

//On importe l'interface contenant toutes les fonctions publiques de hashing. 
//hasher.h contient genere_hash_from_file et genere_hash_from_message, la différence
//entre les deux étant que une prends un nom de fichier et l'autre un string.
#include "hasher.h" 

//L'interface gereoptions contient la méthode get_options qui renvoie une struct
//arguments qui contient un boolean .f qui indique si -f à été fourni ou pas, un
//boolean t, qui indique si -t à été fourni ou pas, ainsi qu'un pointeur char * sur
//le digest fourni (sha1 si non fourni) et un char * sur le message fourni, alloué
//uniquement si -f n'a pas été utilisé.
#include "gereoptions.h"


int main(int argc, char *argv[]) {
    
    //On récupère les options fournies par l'utilisateur.
    struct arguments args = get_options(argc, argv);
    
    //Si on a reçu des fichiers.
    if (args.f) {
        //Depuis la position optind de argv[i] (fournie par getopt), 
        //on parcourt jusqu'à argc, optind ici est l'index de départ
        //des arguments suivant -f.
        for(int i=args.optind; i < argc; i++) {
            //on génère le hash pour chaque fichier, avec le digest fourni.
            genere_hash_from_file(args.digest, argv[i]);
        }
    } else {
        //Si un fichier n'a pas été fourni, on génère le digest
        //avec le message fourni.
        printf("For message \"%s\", with hash function %s \n", args.message, args.digest);
        genere_hash_from_message(args.digest, args.message);
    }
    return 0;
}

//https://www.geeksforgeeks.org/getopt-function-in-c-to-parse-command-line-arguments/
//On mets le code pour gérer les options ici.
//Fichier d'interface.
//Source : 
//https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c

//don't forget to link gereoptions.c and libssl and libcrypto when compiling.
//https://www.gnu.org/software/libc/manual/html_node/Streams.html