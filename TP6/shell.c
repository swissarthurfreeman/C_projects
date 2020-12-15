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

extern pid_t fore_pid, back_pid;

int main() {
    set_masks();

    while (1) {
        
        start:printf("\033[1;31m");
        printf("gordon");    
        printf("\033[1;34m");
        printf("@%s> " , getcwd(NULL, 0));
        printf("\033[0;37m");
        
        //sigaction(SIGINT, &signa, NULL); //permet de Ctrl+C a l'infini.
        //On récupère l'entrée utilisateur avec fgets. (max 1024 charactères)
        char * input = malloc(1024);
        char * res = fgets(input, 1024, stdin);
        
        //certains signaux interrompent fgets, faut faire gaffe.
        if(res == NULL) 
            continue;

        //on enlève newline sinon ça casse tout.
        for(int i = 0; i < 1024; i++) {
            if (input[i] == '\n') {
                if (i == 0)
                    goto start; //cas ou on a juste appuyé sur enter.
                printf("\033[0;37m");
                input[i] = '\0';
            }
        }
        
        int argcount = 0;
        //tableau de chaînes de caractères. (taille fixe à 10)
        char * argvalues[10] = {};
        int redirect_out = 0;
        char * file_name;

        char * token = strtok(input, " ");

        while (token != NULL) {
            
            //redirection de flux.
            if(strcmp(token, ">") == 0) {
                token = strtok(NULL, " ");
                file_name = malloc(sizeof(char)*strlen(token));
                strncpy(file_name, token, sizeof(char)*strlen(token));
                redirect_out = 1;
                printf("%s\n", file_name);
                break;
            }

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
                free(argvalues[argcount - 1]);
                argvalues[argcount - 1] = NULL;
                argcount = argcount - 1;
                back = 1;
            }
            
            pid_t pid = fork();
            //dans l'enfant.
            if (pid == 0) {
                //on configure l'enfant afin d'ingorer SIGINT (si c'est un background task)
                if(back == 1) 
                    config_background_task();
                    
                //si on a une redirection de stdout solved using : 
                //https://stackoverflow.com/questions/19846272/redirecting-i-o-implementation-of-a-shell-in-c
                if(redirect_out == 1) {
                    //literraly changed this line to exactly what stack overflow said.
                    int output_file = open(file_name, O_WRONLY | O_CREAT, 0666);
                    int res = dup2(output_file, STDOUT_FILENO);
                    close(output_file); //and closed output_file, by some magic it works.
                }

                //program launched by execvp sends when exited SIGCHLD (fork).
                int res = execvp(argvalues[0], argvalues);
                
                if(res == -1) {
                    printf("%s \n", strerror(errno));
                    exit(EXIT_FAILURE);
                } else {
                    exit(EXIT_SUCCESS);
                }
            //dans le parent, pid est celui de l'enfant.
            //si back == 1, pid est le background pid 
            } else if(pid > 0) {
                printf("\033[0;37m");
            
                //dans le parent on attends que l'enfant se termine.
                //si il est en foreground.
                if (back == 0) { 
                    fore_pid = pid;
                    //status allows to check state of program at exit.
                    int status = 0;
                    //très bien, faire while not error.
                    dontzombie:sleep(0);
                    int res = waitpid(pid, &status, 0);
                    if (res > 0) {
                        if (WIFSIGNALED(status)) 
                            printf("\nForeground terminated by signal \n");
                        printf("Foreground job exited with code %d\n", status);
                        fore_pid = 0;
                        continue;
                    } else if (res == -1) {
                        printf("Foreground job exit error (status: %d) \n", status);
                        printf("Error : %s \n", strerror(errno));
                        printf("Reattempting exit...\n");
                        //en cas d'erreur on refait un waitpid jusqu'à ce que 
                        //cela passe, on évite les zombies.
                        goto dontzombie;
                        if (WIFSIGNALED(status)) 
                            printf("Foreground terminated by signal \n");
                    } else {
                        //check status.
                        printf("Foreground job exited with code %d \n", status);
                    }
                } else if (back == 1) { 
                    //sinon c'est que c'est un background task.
                    //ici on a le pid de l'enfant pid, on s'en  sert dans 
                    //gordon_child_handler.
                    back_pid = pid;
                }
            }
        }
    }
    return 0;
}