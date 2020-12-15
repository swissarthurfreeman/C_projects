#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <signal.h>
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

extern void config_background_task() {
    int descr = open("/dev/null", O_RDWR);
    //stdin devient descr, on évite les conflits avec le shell.
    int res = dup2(descr, STDIN_FILENO);
    printf("%d", res);
    close(descr);
    
    //On configure les signaux a masquer chez 
    //l'enfant afin que SIGINT soit ignoré.
    //Ctrl+C ne stoppera pas le background task.
    sigset_t child_mask;
    sigemptyset(&child_mask);
    sigaddset(&child_mask, SIGINT);
    sigprocmask(SIG_BLOCK, &child_mask, NULL);
}