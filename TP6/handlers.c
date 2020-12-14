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
        case SIGHUP:
            printf("SIGHUP received, redirecting and exiting...\n");
            if(fore_pid > 0) 
                kill(fore_pid, SIGHUP);
            if(back_pid > 0)
                kill(back_pid, SIGHUP);
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

extern void set_masks() {
    //gérance de SIGCHLD.
    struct sigaction child_death;
    memset(&child_death, 0, sizeof(child_death));
    child_death.sa_sigaction = gordon_child_handler;

    //SA_RESTART fait en sorte que si un signal
    //interromp l'execution d'un système call, alors on relance
    //celui ci une fois que le handler renvoie.
    child_death.sa_flags = SA_SIGINFO; //man sigaction.
    sigaction(SIGCHLD, &child_death, NULL);

    //gérance des autres
    struct sigaction signa;
    memset(&signa, 0, sizeof(signa));
    signa.sa_handler = gordon_handler;
    signa.sa_flags = SA_RESTART;

    sigset_t parent_mask;
    sigemptyset(&parent_mask);
    sigaddset(&parent_mask, SIGTERM);
    sigaddset(&parent_mask, SIGQUIT);
    sigprocmask(SIG_BLOCK, &parent_mask, NULL);
    //sigaction(SIGTERM, &signa, NULL);
    //sigaction(SIGQUIT, &signa, NULL);
    sigaction(SIGINT, &signa, NULL);
    sigaction(SIGHUP, &signa, NULL);
}