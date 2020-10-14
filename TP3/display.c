#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <dirent.h> 
#include "display.h"

void ultrals(char* path) {

    //on récupère les statistiques du fichier pointé par le chemin fourni
    //et on les mets dans la structure raw.
    struct stat raw;
    stat(path, &raw);

    //Récupération des permissions : CONDITION ? INSTRUCTION_SI_OUI : INSTRUCTION_SI_NON;
    char perm[10];
    (S_ISDIR(raw.st_mode)) ?  (perm[0] = 'd') : (perm[0] = '-');
    (raw.st_mode & S_IRUSR) ? (perm[1] = 'r') : (perm[1] = '-');
    (raw.st_mode & S_IWUSR) ? (perm[2] = 'w') : (perm[2] = '-');
    (raw.st_mode & S_IXUSR) ? (perm[3] = 'x') : (perm[3] = '-');
    (raw.st_mode & S_IRGRP) ? (perm[4] = 'r') : (perm[4] = '-');
    (raw.st_mode & S_IWGRP) ? (perm[5] = 'w') : (perm[5] = '-');
    (raw.st_mode & S_IXGRP) ? (perm[6] = 'x') : (perm[6] = '-');
    (raw.st_mode & S_IROTH) ? (perm[7] = 'r') : (perm[7] = '-');
    (raw.st_mode & S_IWOTH) ? (perm[8] = 'w') : (perm[8] = '-');
    (raw.st_mode & S_IXOTH) ? (perm[9] = 'x') : (perm[9] = '-');

    //%24.s imprime les 24 premiers caractères (donc sans le \n renvoyé par ctime)
    printf("%s %ld %.24s %s \n", perm, raw.st_size, ctime(&raw.st_mtime), path);

    //si c'est un dossier, on recommence
    if(perm[0] == 'd') {
        //opendir() returns a pointer of DIR type.   
        //C'est un stream de directory, initialement l'indicateur est à la première entrée du dossier.
        DIR *dr = opendir(path);
        //gestion d'erreur, si on ne peux pas ouvrir un dossier.
        if (dr == NULL) { // opendir returns NULL if couldn't open directory 
            printf("Ouverture de %s impossible, avez vous les droits ?\n", path ); 
        } 

        //Pointer for directory entry 
        struct dirent *de;  

        //Readdir renvoie un nouveau pointeur représentant le prochain dossier dans le 
        //dossier pointé par dr. (on commence à path) https://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
        while ((de = readdir(dr)) != NULL) {
            //Si on est pas sur un nom de directoire local et directoire parent
            //readdir les liste, faut pas sinon on finit jamais...
            //strncmp returns 0 if it matches. 
            if ( strcmp(de->d_name, "..")  ) {
                if( strcmp(de->d_name, ".") ) {               
                    char* newPath;
                    //On crée le nouveau chemin sur lequel appeler ultrals.
                    //le chemin est PATH_FOURNI/NOM_DE_DOSSIER, donc + 1 pour "/""
                    newPath = malloc( strlen(path) + strlen(de->d_name) + 1 );
                    newPath = strcpy(newPath, path); //NewPath devient PATH_FOURNI________________
                    newPath = strcat(newPath, "/"); //NewPath devient PATH_FOURNI/_______________
                    newPath = strcat(newPath, de->d_name); //NewPath devient PATH_FOURNI/NOM_DE_DOSSIER
                    ultrals(newPath); //on recommence récursivement.
                }
            }
        }           
        closedir(dr);
    } 
}

int main(int argc, char* argv[]) {
    //Gestion des paramètres, si on a fourni plus qu'un path, on se barre.
    if(argc > 2) {
        printf("Error, expected only path.\n");
        exit(EXIT_FAILURE);
    }
    //Fonction récursive à laquelle on fournit la racine.
    ultrals(argv[1]);  
    return 0;
}