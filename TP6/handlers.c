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

extern void gordon_handler(int signum) {
    switch(signum) {
        case SIGINT:
            //ici il faut rediriger vers le processus principal.
            printf("Received Sigint\n");
            break;
    }
}
//this fucntion handles everything to do with child stopping (SIGCHLD)(special handler)
void gordon_child_handler(int signum, siginfo_t *siginfo, void* unused) {
    
    printf("Hello");
}   