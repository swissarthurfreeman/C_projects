#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>

//fonction qui cherche dans les entrées de dir_name le fichier de taille
//maximale
static void find(const char *dir_name , char *result , int *value ) {
    DIR *d = opendir( dir_name );
    struct dirent *entry;
    const char *d_name;
    if (!d) {
        fprintf(stderr , "Cannot open ’%s’: %s\n", dir_name , strerror (errno));
        return;
    }

    /*
    struct dirent {
        ino_t d_ino;  Inode number
        off_t d_off; 
        unsigned short d_reclen; longueur du record
        unsigned char d_type; Type de fichier (pas soutenu par tout filesystem)
        char dname[256]; nom du fichier.
    }
    
    */
    //on explore tous les sous directoires et fichiers (toutes les entrées)
    while( (entry = readdir(d)) != NULL ) {
        //nom du directoire. (-> car entry est un pointeur)
        d_name = entry->d_name;

        char path[ PATH_MAX ];

        //on écrit dir_name/d_name dans path
        snprintf(path , PATH_MAX ,"%s/%s", dir_name , d_name);
        
        //si c'est un fichier régulier
        if( entry ->d_type & DT_REG ) {
            struct stat s;
            //on popule la structure stat.
            if( stat( path , &s ) >= 0 ) {
                //taille du fichier en bytes.
                int current = s.st_size;
                //si la taille du fichier est plus grande que la taille précédente
                if( current > *value ) {
                    //on change value
                    *value = current;
                    //on copie le path dans result
                    strncpy( result , path , PATH_MAX );
                }
            }
        //si c'est un directoire
        } else if( entry ->d_type & DT_DIR ) {
            char res[PATH_MAX];
            int s = 0;
            find( path , (char*) res , &s );
            if( s > *value ) {
                *value = s;
                strncpy( result , res , PATH_MAX );
            }
        }
    }

    closedir(d);
}

int main ( int argc , char ** argv ) {
    int value = 0;
    char res[PATH_MAX];

    //On ne filtre pas '.' et '..' il vas continuer
    //a l'infinit. 
    find( argv [1], (char*) res , &value);
    printf( "Found: %s (%d).\n", res , value );
    return EXIT_SUCCESS;
}



/*

C'est pas compatible POSIX car cela dépends du filesystem.
On peux utiliser un stat afin de récupérer le type du fichier
en utilisant la propriété st_mode du fichier, et en utilisant
les macros : 

S_ISDIR, S_ISREG..
Et on ferait st_mode & S_ISDIR
Si c'est différent de 0, c'est que le bit de directoire est activé.

*/