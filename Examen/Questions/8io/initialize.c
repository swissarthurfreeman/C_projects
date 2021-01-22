#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main( int argc , char ** argv ) {
    //nom du fichier
    char* dataFile = argv[1];
    int fd = open( dataFile , O_WRONLY|O_CREAT|O_TRUNC , 0600 );
    //on l'ouvre et on écrit les montants dans chaque compte.
    int accounts [] = { 100, 20, 50, 1000 , 5 };
    //sizeof(int)*5, car on a 5 comptes.
    write( fd , accounts , sizeof(int) * 5 );
    close(fd);
    return EXIT_SUCCESS ;
}
