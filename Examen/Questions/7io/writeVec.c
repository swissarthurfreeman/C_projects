#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

//stocke la liste de doubles pointée par vec dans le fichier
//dont le descripteur de fichier est fd.
int storeSingle ( double *vec , int length , int fd ) {
    //on écrit la longueur de la ligne
    write( fd , (char*) &length , sizeof(int) );
    
    //on écrit la ligne en charactères, on caste un pointeur double
    //en un pointeur de char, ce qui ne change rien... c'est toujours
    //la même addresse. On écrit tous les doubles. 
    write( fd , (char*) vec , length * sizeof(double) );
    return 1;
}

//fonction qui stocke les lignes de la matrice définie dans le main
//dans un fichier.
int store( double **vec , int *lengths , int n, char *fname ) {
    //on ouvre le fichier vec.dat en écriture (on le crée si il n'existe pas)
    //avec droits read write utilisateur. (on est pas garantis de les avoir, le masque doit être bon) 
    //par défaut le masque de l'utilisateur est 022, e.g. ~022 = 755
    //donc les permissions sur le fichier seront rw------- == 600 & 755 = 600 = rw-------
    //on peut voir le masque avec umask.
    int fd = open( fname , O_WRONLY | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR );
    int iVec = 0;
    //iVec varie de 0 a 2.
    for( iVec = 0; iVec < n; iVec ++ ) {
        storeSingle ( vec[iVec], lengths[iVec], fd );
    }
    close(fd);
    return 1;
}

int main( int argc , char ** argv ) {
    int lengths [] = { 2, 8, 5 };
    double v1[] = {0.1 , 0.5};
    double v2[] = {1.0 , 2, 3, 4, 5, 6, 7, 8};
    double v3[] = { -100.0 , -10, 1, 10, 100};

    //e.g. tableau de pointeurs vers des doubles
    //un pointeur vers un double est un tableau
    //vecs[1] pointe sur {1.0 , 2, 3, 4, 5, 6, 7, 8}
    //et on accède a la i-ième case en faisant vecs *(vec[1] + sizeof(double)*i)
    double *vecs [3];
    vecs [0] = v1;
    vecs [1] = v2;
    vecs [2] = v3;
    
    store( vecs , lengths , 3, "vec.dat" );
    exit(EXIT_SUCCESS);
}

/*
1. Décrivez en détail le code et son fonctionement.

(...)

2. Pour lire les vecteurs, il faut connaître à l’avance le nombre de vecteurs à lire.
Comment faire pour lire tous les vecteurs présents, sans en connaître le nombre
à l’avance ? (vous pouvez changer le format)

On utilise une fonction qui parcoure le fichier une première fois et nous dit combien
de vecteurs sont présents : 

//Question 2)
int longueur(char * name) {
    int fd = open(name, O_RDONLY);
    //on lit la valeur de la longueur
    //on la lit.
    int length;
    int count = 0;
    while(read(fd, &length, sizeof(int) != 0)) {
        count++;
        read(fd, NULL, length*sizeof(double));
    }
}

3. Comment faire pour ne lire qu’un seul vecteur (par exemple le troisième de la liste) ? 
Présentez une solution en pseudo-code qui ne nécéssite pas un changement de format. 
Indiquez les appels systèmes et fonctions employées.

//Question 3) et on commente...
double * readSingle(const char * name, const int i) {
    int fd = open(name, O_RDONLY);
    int length;
    int count = 0;
    double * values;
    while(count < i) {
        read(fd, &length, sizeof(int));
        if(count == i - 1) {
            values = calloc(length, sizeof(double));
            read(fd, values, sizeof(double)*length);    
            break;
        } 
        read(fd, NULL, sizeof(double)*length);
        count++;
    }
    return values;
}
*/