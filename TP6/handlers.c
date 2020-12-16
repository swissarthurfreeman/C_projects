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

pid_t fore_pid; 
pid_t back_pid;

//Gordon handler handles SIGINT and SIGHUP.
extern void gordon_handler(int signum) {
    switch(signum) {
        case SIGINT:
            //ici il faut rediriger vers le processus principal.
            //si y'a un foreground task.
            if(fore_pid > 0) {
                kill(fore_pid, SIGINT);
                write(STDOUT_FILENO, "\nKilled foreground\n", 20);
                break;
            }
            //sinon c'est que y'a pas de foreground task, donc on fait rien.
            //ctrl+C ne ferme pas le shell dans le shell.
            break;
        //If sighup is received, we must redirecrt this to foreground
        //and background processes. 
        case SIGHUP:
            write(STDOUT_FILENO, "SIGHUP received, redirecting and exiting...\n", 45);
            if(fore_pid > 0) 
                kill(fore_pid, SIGHUP);
            if(back_pid > 0)
                kill(back_pid, SIGHUP);
            //after which we exit.
            exit(EXIT_SUCCESS);
            break;
    }
}
//this function handles everything to do with child stopping (SIGCHLD)(special handler)
//quand ceci est appelé c'est qu'on a reçu SIGCHLD.
void gordon_child_handler(int signum, siginfo_t *siginfo, void* unused) {
    //Si c'est lep rocessus background.
    if (back_pid == siginfo->si_pid) {
        int status;
        //allows to unzombie the terminated child.
        if(back_pid != waitpid(siginfo->si_pid, &status, WNOHANG)) 
            return;
        
        write(STDOUT_FILENO, "\nBackground Job Exited\n", 23);
        back_pid = 0;
    }
}

//set_masks configures the masks and special handlers of the shell.
extern void set_masks() {
    //gérance de SIGCHLD.
    struct sigaction child_death;
    memset(&child_death, 0, sizeof(child_death));
    child_death.sa_sigaction = gordon_child_handler;

    //SA_SIGINFO flag allows to use sa_sigaction member
    //as handler. child_death is only for SGCHLD.
    child_death.sa_flags = SA_SIGINFO; 
    sigaction(SIGCHLD, &child_death, NULL);

    //signa is used to manage SIGINT and SIGHUP signals
    //which require a normal handler.
    struct sigaction signa;
    memset(&signa, 0, sizeof(signa));
    signa.sa_handler = gordon_handler;
    signa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &signa, NULL);
    sigaction(SIGHUP, &signa, NULL);

    //parent_mask is configured to ignore
    //SIGTERM and SIGQUIT signals.
    sigset_t parent_mask;
    sigemptyset(&parent_mask);
    sigaddset(&parent_mask, SIGTERM);
    sigaddset(&parent_mask, SIGQUIT);
    sigprocmask(SIG_BLOCK, &parent_mask, NULL);   
}