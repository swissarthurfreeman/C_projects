#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

//loadSingle renvoie un pointeur vers le tableau de doubles
//qui représente la ligne de longueur length
//(c'est écrit linéairement dans le fichier)
double * loadSingle ( int fd , int *length ) {
    double *v;
    int l;
    //on récupère la taille du vecteur
    //on déplace le curseur dans le descripteur de fichier
    //a chaque fois ! 
    //read rempli le buffer pointé par length a chaque fois
    read( fd , length , sizeof(int) );
    //on compte le nombre de bytes a lire pour ce vecteur
    l = (* length) * sizeof(double);
    //on alloue un tableau de doubles de longueur l
    v = (double*) malloc(l);
    //on le remplit avec la longueur appropriée.
    read( fd , v, l );
    return v;
}


int load( int n, double *** vecs , int ** lengths , char *fname ) {
    //ici vecs est l'addresse de la matrice en mémoire.
    //on ouvre le fichier en lecture.
    int fd = open( fname , O_RDONLY );
    //on se mets bien au début ! 
    lseek( fd , 0, SEEK_SET );
    int iVec = 0;
    //*vecs est la matrice, un pointeur vers le début du tableau de tableaux,
    //on alloue 3 pointeurs vers des doubles (qui sont des tableaux) 
    //Calloc alloue un tableau de n pointeurs vers des doubles.
    //et renvoie le pointeur vers ce dernier, donc calloc renvoie bien
    //un double pointeur (ce qui correspond a *vecs)
    *vecs = calloc( n, sizeof(double*) );
    //lengths est un tableau qui contiendra la longueur de chaque vecteur (tableau de doubles) 
    *lengths = calloc( n, sizeof(int) );
    //dans chaque case du tableau de pointeurs de doubles
    //on alloue le pointeur vers un tableau de doubles.
    //val[i] = *(val + i*sizeof(type(val)))
    for( iVec = 0; iVec < n; iVec ++ ) {
        //(* vecs)[iVec] == *(*vecs + iVec*sizeof(double*) )
        //&( (* lengths)[iVec] ) est l'addresse a laquelle est placée
        //la longueur de la ligne iVec.
        //(*vecs)[iVec] === *double
        //&( (* lengths)[iVec] ) est l'addresse de la case [ivec] du tableau
        //de longueurs.
        (* vecs)[iVec] = loadSingle ( fd , &( (* lengths)[iVec] ) );
    }
    close(fd);
    return 1;
}

//Question 2)
int longueur(char * name) {
    
    int fd = open(name, O_RDONLY);
    //on se mets bien au début ! 
    lseek( fd , 0, SEEK_SET );
    //on lit la valeur de la longueur
    //on la lit.
    int length;
    int count = 0;
    //si le file offset est au dela de la fin
    //du fichier, read retourne 0
    while(read(fd, &length, sizeof(int) != 0)) {
        count++;
        read(fd, NULL, length*sizeof(double));
    }
}

//Question 3)
double * readSingle(const char * name, const int i) {
    int fd = open(name, O_RDONLY);
    //on se mets bien au début ! 
    lseek( fd , 0, SEEK_SET );
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

int main( int argc , char ** argv ) {
    //matrice de doubles.
    double ** vecs;
    //tableau de longueurs.
    int *lengths;
    int iVec;
    //on charge les valeurs de la matrice contenues
    //dans vec.dat, la valeur de lengths sera modifiée 
    //(chaque ligne a une longueur différente) lengths est tableau de pointeurs.
    //vecs pointera sur le début de la matrice. (vecs est pointeur vers pointeur
    //donc tableau de pointeurs donc tableau de tableaux.)
    //on passe &vecs, car C copie les valeurs, il nous faut l'addresse, idem avec &lengths.
    int n = longueur("vec.dat");
    load( 3, &vecs , &lengths , "vec.dat" );
    //on affiche les valeurs dans la matrice.
    for( iVec = 0; iVec < 3; iVec ++ ) {
        int i;
        for( i = 0; i < lengths[iVec ]; i++ ) {
            printf("%f ", vecs[iVec ][i] );
        }
        printf("\n");
    }
    exit( EXIT_SUCCESS );
}
