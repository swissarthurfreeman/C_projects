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

//To Do : 
/*
1) Réparer problème ou Ctrl+C stoppe une tâche en fond.
2) Réparer gestion de SIGHUP qui ne fait pas ce qu'il faut
    -Cas ou on a une tâche en fond
    -Cas ou on a deux tâches en fond
3) Réparer les erreurs par rapport aux variables globals fore_pid et back_pid.
*/

extern pid_t fore_pid, back_pid;
int main() {
    //gérance de SIGCHLD.
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = gordon_child_handler;

    //SA_RESTART fait en sorte que si un signal handler
    //interromp l'execution d'un système call, alors on relance
    //celui ci une fois que le handler renvoie.
    sa.sa_flags = SA_SIGINFO | SA_RESTART; //man sigaction.
    sigaction(SIGCHLD, &sa, NULL);

    //gérance des autres
    struct sigaction signa;
    signa.sa_handler = gordon_handler;
    signa.sa_flags = SA_RESTART;

    sigaction(SIGTERM, &signa, NULL);
    sigaction(SIGQUIT, &signa, NULL);
    sigaction(SIGINT, &signa, NULL);
    sigaction(SIGHUP, &signa, NULL);

    start:printf("gordon@%s> ", getcwd(NULL, 0));

    while (1) {
        
        sigaction(SIGINT, &signa, NULL); //permet de Ctrl+C a l'infini.
        //On récupère l'entrée utilisateur avec fgets. (max 1024 charactères)
        char * input = malloc(1024);
        char * res = fgets(input, 1024, stdin);
        
        //certains signaux interrompent fgets, faut faire gaffe.
        if(res == NULL) { 
            continue;
        }

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
            //on écrit gordon@path>nom_programme [options] &
            int back = 0;
            if(argvalues[argcount - 1][0] == '&') {
                argvalues[argcount - 1][0] = '\0';
                printf("%s\n", argvalues[0]);
                back = 1;
            }
            pid_t pid = fork();
            //dans l'enfant.
            if (pid == 0) {
                if(back == 1) {
                    int descr = open("/dev/null", O_RDWR);
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
            //si back == 1, pid est le background pid 
            } else if(pid > 0) {
                
                //dans le parent on attends que l'enfant se termine.
                if (back == 0) {
                    fore_pid = pid;
                    //back_pid = 0;
                    //status allows to check state of program at exit.
                    int status = 0;
                    dontzombie:sleep(0);
                    int res = waitpid(pid, &status, 0);
                    if (res > 0) {
                        if (WIFSIGNALED(status)) 
                            printf("\nForeground terminated by signal \n");
                        printf("Foreground job exited with code 0\n");
                        fore_pid = 0;
                        goto start;

                    } else if (res == -1) {
                        printf("Foreground job exit error \n");
                        printf("Error : %s \n", strerror(errno));
                        //en cas d'erreur on refait un waitpid jusqu'à ce que 
                        //cela passe, on évite les zombies.
                        goto dontzombie;
                        if (WIFSIGNALED(status)) 
                            printf("Foreground terminated by signal \n");
                    } else {
                        //check status.
                        printf("Foreground job exited with code %d \n", status);
                    }
                } else if (back == 1) { //sinon c'est que c'est un background task.
                    //ici on a le pid de l'enfant pid, on s'en  sert dans 
                    //gordon_child_handler.
                    back_pid = pid;
                }
            }
        }
        printf("gordon@%s> ", getcwd(NULL, 0));
    }
    return 0;
}