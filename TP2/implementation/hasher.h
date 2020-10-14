/************************************************
*  Interface d'hashing                          *
*  Auteur : Arthur Freeman, Date : 11/10/2020.  *
*************************************************/ 

//Fonction qui génère un hash à partir d'un message via openssl / libcrypto.
extern void genere_hash_from_message(char hashType[], char message[]);

//Fonction qui génère un hash à partir d'un nom de fichier, on le lit dans genere_hash_from_file.
extern void genere_hash_from_file(char hashType[], char FILENAME[]);