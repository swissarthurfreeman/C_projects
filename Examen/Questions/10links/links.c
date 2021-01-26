#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc , char *argv []) {
    if (argc != 3) {
        printf("Invalid number of arguments:\nUsage:\n\t%s source dest\n", argv [0]);
        return 0;
    }

    char *in_dir = argv [1];
    char *out_dir = argv [2];

    struct stat stats;

    //stat vas déréférencer le lien symbolique
    if(stat(out_dir , &stats) < 0) {
        perror(out_dir);
        exit( EXIT_FAILURE );
    }

    //si out_dir n'est pas un dossier
    if ( !( stats.st_mode & S_IFDIR) ) {
        fprintf(stderr , "%s: is not a directory\n", out_dir);
        exit( EXIT_FAILURE );
    }

    //on ouvre le dossier, c'est un stream d'entrées
    DIR *d = opendir(in_dir);
    if (! d) {
        fprintf(stderr , "Cannot open directory ’%s’: %s\n", in_dir ,
        strerror(errno));
        exit( EXIT_FAILURE );
    }

    struct dirent *entry;
    //on parcourt toutes les entrées (readdir renvoie un dirent * conenant le nom entre autres)
    while( (entry = readdir(d)) != NULL ) {
        //on omets le dossier courant et parent (afin de ne pas boucler)
        if (strcmp(entry->d_name , "..") != 0 && strcmp(entry->d_name , ".") != 0) {

            char path_in[ PATH_MAX ];

            //snprintf écrit dans path_in in_dir/entry->d_name (e.g. c'est une entrée)
            int res = snprintf(path_in , PATH_MAX , "%s/%s", in_dir , entry ->d_name);
            
            if(res >= PATH_MAX )
                fprintf(stderr , "Input path length is too long.\n");

            char path_out [ PATH_MAX ];
            
            //on écrit dans path_out out_dir/entry->d_name (e.g. c'est un nouveau répertoire avec ces entrées)
            res = snprintf(path_out , PATH_MAX , "%s/%s", out_dir , entry ->d_name);
            
            if(res >= PATH_MAX )
                fprintf(stderr , "Output path length is too long.\n");

            //on crée un lien dur entre path_in (le path vers le dossier d'entrée)
            //et path out (le path vers le dossier de sortie)
            if(link(path_in , path_out) < 0)
                perror("not possible to link");
        }
    }

    if( closedir (d) ) {
        fprintf(stderr , "Could not close ’%s’: %s\n", in_dir , strerror (errno));
        exit ( EXIT_FAILURE );
    }

}

/*
Questions : (cf. man link)

1. Que fait le programme suivant ?

Le programme suivant prends deux paramètres in_dir et out_dir qui sont des noms de dossiers,
et crée un lien dur de out_dir/nom_entree vers in_dir/nom_entree. e.g. on crée un nouveau lien
dur dans out_dir/fichier et in_dir/fichier avec chaque entrée dans in_dir. 

Rappel : dans le système de fichier un lien dur /tmp/file est un "pointeur" sur un inode qui "pointe" vers 
des données. En créant un deuxième lien dur, on incrémente le nombre de liens vers cet inode. 

2. Si le deuxième paramètre du programme (argv[2]) correspond à un lien symbolique, que ce passe-t-il ?

Cela revient a dire que out_dir est un lien symbolique. Un lien symbolique est un lien dont l'inode
pointe vers des données qui sont un lien dur. Link vas déréférencer le lien en faisant l'expérience,
même si ce n'est pas explicitement dit dans le manuel certainement que link utilise stat et déréférence
automatiquement. Sinon peut être que c'est pas défini.

3. Si le dossier passé en premier paramètre (argv[1]) contient des dossiers, que ce
passe-t-il ?

Cela vas générer l'erreur EPERM, e.g. oldpath est un dossier. On ne peux pas avoir plus
de 1 lien dur vers un dossier, cela fait partie des limitations. Le programme vas abandonner
les dossiers et ne créera pas de liens (perror ne fait pas d'exit)

4. Si le dossier passé en premier paramètre (argv[1]) contient des liens symboliques,
que ce passe-t-il ? Expliquer avec un schéma.

cf. Notes man link

Ce qui vas se passer c'est que étant donné que oldpath est un lien symbolique, et que link
traite ses paramètres comme si c'était des liens durs et ne déréférence pas, (alors qu'il devrait selon les notes de la man)
link vas créer un lien dur newpath vers le lien symbolique oldpath, e.g. vers l'inode de oldpath, dont
les données sont un lien dur. Donc en fait newpath sera un lien symbolique mais ne serait pas garanti
de marcher si il était relatif. 

*/