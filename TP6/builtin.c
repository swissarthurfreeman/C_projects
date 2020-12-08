#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "builtin.h"

extern char * cd_gordon(char * to) {
    int res = chdir(to);
    if (res == -1)
        printf("%s\n", strerror(errno));
        
    char * current = getcwd(NULL, 0);
    printf( "%s\n",  current);
}

extern int exit_gordon() {
    //cette fonction doit terminer le shell proprement
    //c.à.d fermer tous les jobs, vérifier que c'est bon.
    printf("Exiting... \n");
    exit(EXIT_SUCCESS);
}