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

//cd_gordon takes new absolute or relative path
//uses chdir syscall.
extern char * cd_gordon(char * to) {
    int res = chdir(to);
    if (res == -1)
        printf("%s\n", strerror(errno));
    
    char * current = getcwd(NULL, 0);
    printf( "%s\n",  current);
}

//exit gordon sends SIGHUP to all child processes
//and stops shell correctly avoiding zombies.
extern int exit_gordon() {
    //cette fonction termine le shell proprement.
    //sighup terminera tous les processus en background.
    kill(getpid(), SIGHUP);
    printf("Exiting... \n");
}

//configures the child mask and redirects his standard input
//to read from /dev/null, this avoids conflicts with shell.
//mask ignores SIGINT in order not to be prone to Ctrl+C.
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