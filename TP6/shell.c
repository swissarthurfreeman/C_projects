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
#include "handlers.h"

int main() {
    struct sigaction sa;
    sa.sa_sigaction = gordon_child_handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGCHLD, &sa, NULL);
    
    //sa.sa_handler = gordon_handler; 
    //sa.sa_sigaction = SA_SIGINFO;
    //sigemptyset(&sa.sa_mask);
    //sigaddset(&sa.sa_mask, SIGUSR1);
    //sigaddset(&sa.sa_sigaction, SA_SIGINFO);
    
    start:printf("gordon@%s>", getcwd(NULL, 0));
    while (1) {
        //On récupère l'entrée utilisateur avec fgets. (max 1024 charactères)
        char * input = malloc(1024);
        fgets(input, 1024, stdin);
        //on enlève newline sinon ça casse tout.
        for(int i = 0; i < 1024; i++) {
            if (input[i] == '\n') {
                if (i == 0)
                    goto start; //cas ou on a juste appuyé sur enter.
                input[i] = '\0';
            }
        }
        
        int argcount = 0;
        //tableau de chaînes de caractères. (taille fixe à 10)
        char * argvalues[10] = {};
        char * token = strtok(input, " ");
        
        while (token != NULL) {
            argvalues[argcount] = malloc(sizeof(char)*strlen(token));
            strncpy(argvalues[argcount], token, strlen(token)*sizeof(char));
            token = strtok(NULL, " ");
            argcount += 1;
        }

        //ici on execute ce qui était fourni en paramètre
        if (strcmp(argvalues[0], "cd") == 0) {
            printf("Exectue cd \n");
            cd_gordon(argvalues[1]);
        } else if (strcmp(argvalues[0], "exit") == 0) {
            exit_gordon();
        } else {
            //si c'est un programme a executer en fond
            int back = 0;
            if(argvalues[0][strlen(argvalues[0]) - 1] == '&') {
                argvalues[0][strlen(argvalues[0]) - 1] = '\0';
                printf("%s\n", argvalues[0]);
                back = 1;
            }
            pid_t pid = fork();
            //dans l'enfant.
            if (pid == 0) {
                if(back == 1) {
                    int descr = open("/dev/null", O_WRONLY);
                    //stdin devient descr, on évite les conflits avec le shell.
                    dup2(descr, STDIN_FILENO);
                    dup2(descr, STDOUT_FILENO);
                    close(descr);
                }
                //execvp sends SIGCHLD.
                int res = execvp(argvalues[0], argvalues); 
                if (res == -1) {
                    printf("%s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                } else {
                    exit(EXIT_SUCCESS);
                }
            //dans le parent, pid est celui de l'enfant.
            } else if(pid > 0) {
                
                //dans le parent on attends que l'enfant se termine.
                if (back == 0) {
                    //status allows to check state of program at exit.
                    int status = 0;
                    int res = waitpid(pid, &status, 0);
                    if (res == 0) {
                        printf("Foreground job exited with code 0\n");
                    } else if (res == -1) {
                        printf("Foreground job exited with code -1\n");
                    } else {

                    }
                } 
            }
        }
        printf("gordon@%s>", getcwd(NULL, 0));
    }
    return 0;
}