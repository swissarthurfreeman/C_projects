/*
Using an OpenSSL message digest/hash function, consists of the following steps:

Create a Message Digest context
Initialise the context by identifying the algorithm to be used (built-in algorithms are defined in evp.h)
Provide the message whose digest needs to be calculated. 
Messages can be divided into sections and provided over a number of calls to the library if necessary
Caclulate the digest
Clean up the context if no longer required
Message digest algorithms are identified using an EVP_MD object. 
These are built-in to the library and obtained through appropriate library calls (e.g. such as EVP_sha256() or EVP_sha512()).q
*/

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

int main(int argc, char *argv[])
    {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    char mess[] = "Le manuel disait: Nécessite Windows 7 ou mieux. J'ai donc installé Linux";
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len, i;

    if (argv[1] == NULL) { //Si on a pas fourni de fonction de hash.
        printf("Usage: mdtest digestname\n"); //On dit à l'utilisateur d'en fournir.
    	exit(1);
    }

    md = EVP_get_digestbyname(argv[1]); //Sinon, on vérifie qu'elle existe.
    if (md == NULL) { //Si elle existe pas, on exit.
    	printf("Unknown message digest %s\n", argv[1]);
    	exit(1);
    }

    mdctx = EVP_MD_CTX_new(); //alloue et renvoie un "context de digest"
    EVP_DigestInit_ex(mdctx, md, NULL); //configure le context de digest à utiliser un digest du ENGINE impl.
    EVP_DigestUpdate(mdctx, mess, strlen(mess)); //hash nb octets de données dans le context mdctx.
    EVP_DigestFinal_ex(mdctx, md_value, &md_len); //récupère la valeur du digest de mdctx et la place dans md_value
    EVP_MD_CTX_free(mdctx);

    printf("Digest is: ");
    for (i = 0; i < md_len; i++)
    	printf("%02x", md_value[i]);

    printf("\n");

    exit(0);
}
