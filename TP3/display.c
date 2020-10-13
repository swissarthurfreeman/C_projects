#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <dirent.h> 

//structure regroupant les informations
//à extraire de stat.
struct infos {
    char* type;
    char perm[10];
    int* size;
    char* date;
    char* nom;
};

int main(int argc, char* argv[]) {
    
    //Gestion des paramètres, si on a fourni plus qu'un path, on se barre.
    if(argc > 2) {
        printf("Error, expected only path.\n");
        exit(EXIT_FAILURE);
    }
    
    struct infos data;
    data.type = malloc(sizeof(char));
    *data.type = 'f';

    char* path = argv[1];
    printf("Source Path is : %s\n", path);

    struct stat raw;
    //on récupère les statistiques du fichier pointé par le chemin fourni
    //et on les mets dans la structure raw.
    stat(argv[1], &raw);

    //printf("%ld \n", raw.st_size);
    data.size = malloc(sizeof(char));
    *data.size = raw.st_size;
    

    /*if((raw.st_mode & S_IFMT) == S_IFDIR) {
        //printf("C'est un répertoire. \n");
        *data.type = 'd';
    } else {
        printf("C'est un fichier. \n");
    }*/

    //récupération des permissions    
    (S_ISDIR(raw.st_mode)) ?  (data.perm[0] = 'd') : (data.perm[0] = '-');
    (raw.st_mode & S_IRUSR) ? (data.perm[1] = 'r') : (data.perm[1] = '-');
    (raw.st_mode & S_IWUSR) ? (data.perm[2] = 'w') : (data.perm[2] = '-');
    (raw.st_mode & S_IXUSR) ? (data.perm[3] = 'x') : (data.perm[3] = '-');
    (raw.st_mode & S_IRGRP) ? (data.perm[4] = 'r') : (data.perm[4] = '-');
    (raw.st_mode & S_IWGRP) ? (data.perm[5] = 'w') : (data.perm[5] = '-');
    (raw.st_mode & S_IXGRP) ? (data.perm[6] = 'x') : (data.perm[6] = '-');
    (raw.st_mode & S_IROTH) ? (data.perm[7] = 'r') : (data.perm[7] = '-');
    (raw.st_mode & S_IWOTH) ? (data.perm[8] = 'w') : (data.perm[8] = '-');
    (raw.st_mode & S_IXOTH) ? (data.perm[9] = 'x') : (data.perm[9] = '-');
    
    //%24.s imprime les 24 premiers caractères (donc sans le \n renvoyé par ctime)
    printf("%s %d %.24s %s \n", data.perm, *data.size, ctime(&raw.st_mtime), path);

    //Fonction récursive
    struct dirent *de;  // Pointer for directory entry 
  
    // opendir() returns a pointer of DIR type.   https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
    DIR *dr = opendir("./testdir"); 
  
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 
  
    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
    // for readdir() 
    while ((de = readdir(dr)) != NULL) 
            printf("%s\n", de->d_name); 
  
    closedir(dr);  
    return 0;
}