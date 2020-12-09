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

pid_t fore_pid; 
pid_t back_pid;

extern void gordon_handler(int signum) {
    switch(signum) {
        case SIGINT:
            //ici il faut rediriger vers le processus principal.
            //si y'a un foreground task.
            if(fore_pid > 0) {
                printf("Killed foreground \n");
                kill(fore_pid, SIGINT);
                break;
            }
            //sinon c'est que y'a pas de foreground task, donc on fait rien.
            //ctrl+C ne ferme pas le shell dans le shell.
            break;
        case SIGTERM: //on ne fait rien avec sigterm.
            break;
        case SIGQUIT:
            printf("\ngordon@%s> ", getcwd(NULL, 0));
            break;
        case SIGHUP:
            printf("SIGHUP\n");
            printf("%d, %d", fore_pid, back_pid);
            if(fore_pid > 0) 
                kill(fore_pid, SIGKILL);
            if(back_pid > 0)
                kill(back_pid, SIGKILL);
            exit(EXIT_SUCCESS);
            break;
    }
}
//this function handles everything to do with child stopping (SIGCHLD)(special handler)
//quand ceci est appelé c'est qu'on a reçu SIGCHLD.
void gordon_child_handler(int signum, siginfo_t *siginfo, void* unused) {
    //Si c'est lep rocessus background.s
    switch(signum) {
        case SIGCHLD:
            if (back_pid == siginfo->si_pid) {
                int status;
                //allows to unzombie the terminated child.
                waitpid(siginfo->si_pid, &status, 0);
                write(STDOUT_FILENO, "\nBackground Job Exited", 23);
                back_pid = 0;
                return;
            }
            break;
    }
}