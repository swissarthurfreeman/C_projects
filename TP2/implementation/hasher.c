/************************************************
*  Implémentation de l'interface hasher         *
*  Auteur : Arthur Freeman, Date : 11/10/2020.  *
*************************************************/ 

#include <stdio.h> //Pour printf, fseek
#include <openssl/evp.h> //Pour les fonctions de hashing.
#include <string.h> //Pour strlen.
#include "hasher.h" //Interface à implémenter.

//Implémentation de genere_hash_from_message.
void genere_hash_from_message(char hashType[], char message[]) {
    //Inspiré de l'exemple de la man EVP_Digest.
    EVP_MD_CTX *mdctx; 
    const EVP_MD *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len, i;
    md = EVP_get_digestbyname(hashType); 
    mdctx = EVP_MD_CTX_new(); //alloue et renvoie un "context de digest"
    EVP_DigestInit_ex(mdctx, md, NULL); //configure le context de digest à utiliser un digest du ENGINE impl.
    EVP_DigestUpdate(mdctx, message, strlen(message)); //hash nb octets de données dans le context mdctx.
    EVP_DigestFinal_ex(mdctx, md_value, &md_len); //récupère la valeur du digest de mdctx et la place dans md_value
    EVP_MD_CTX_free(mdctx);

    //On imprime la valeur du digest.
    for (i = 0; i < md_len; i++)
    	printf("%02x", md_value[i]);
    printf("\n");
}

//Fonction privée qui lit le fichier fourni et le renvoie 
//dans une chaîne de caractères.
static char* get_text(char FILENAME[]) {
   
    char * buffer = 0;
    long length;
    //fopen associe un stream au fichier fourni.
    FILE * f = fopen(FILENAME, "rb");

    //Si le fichier existe.
    if (f) {
        //fseek mets l'indicateur de position 
        //fseek(FILE *stream, long offset, int whence)
        //on peut voir un stream comme un flux de texte qu'on lit en fonction
        //de la position, tel un tableau.
        //on mets l'indicateur de position du fichier à la fin (SEEK_END)
        fseek(f, 0, SEEK_END);

        //ftell récupère la valeur de l'indicateur de position du stream f.
        //c.à.d ici, on récupère la position de fin du fichier et donc sa longueur !
        length = ftell(f);
        //On se repositionne en début de fichier.
        fseek(f, 0, SEEK_SET);
        //On alloue le buffer nécessaire pour pouvoir stocker le fichier
        //malloc prends un nombre d'octets, et length est un nombre de caractères
        //ASCII, qui se codent chacun sur un octet, c'est parfait !
        buffer = malloc(length); 

    //Si on a réussi à allouer.
    if (buffer) {
        //On lit depuis l'indicateur de position de f le stream f, sur length items de données
        //chacun de taille 1 octet. Ainsi, on lit tout le fichier, et on stocke à la position
        //pointée par buffer. fread(void *ptr, size_t size, size_t nmemb, FILE * stream)
        fread(buffer, 1, length, f);
    }
    //On ferme le fichier en flushant le stream. (on ferme la connection)
    fclose(f);
    }
    //On renvoie l'adresse du début du texte que l'on a alloué
    //sur le tas.
    return buffer;
}

void genere_hash_from_file(char hashType[], char FILENAME[]) {
    printf("For file: %s\n", FILENAME);
    printf("Digest is: %s\n", hashType);
    char * buffer;
    buffer = get_text(FILENAME);
    //On réutilise genere_hash_from_message mais sur le fichier stocké sur le tas.
    //le désavantage étant que si le fichier fais plusieurs Gio, on risque d'avoir
    //des problèmes de mémoire.
    genere_hash_from_message(hashType, buffer);
}