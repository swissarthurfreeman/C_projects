#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

void print_help() {
    printf("\n");
    printf("    Format : cmd l_type start length [ whence ( optional ) ]\n");
    printf("    'cmd ' --- 'g ' ( F_GETLK ) , 's ' ( F_SETLK ) , or 'w ' ( F_SETLKW )\n"); 
    printf("    'l_type ' --- 'r ' ( F_RDLCK ) , 'w ' ( F_WRLCK ) , or 'u ' ( F_UNLCK )\n");
    printf("    'start ' --- lock starting offset\n"); 
    printf("    'length ' --- number of bytes to lock\n");
    printf("    'whence ' --- 's' ( SEEK_SET , default ) , 'c' ( SEEK_CUR ) , or 'e' ( SEEK_END ) \n");
    printf("\n");
}

typedef struct {
    __u_int cmd;
    __u_int l_type;
    __u_int start;
    __u_int length;
    __u_int whence;
} arguments_t;

typedef struct {
    char cmd;
    char l_type;
    __u_int start;
    __u_int length;
    char whence;
} user_arguments_t;

int main(int argc, char* argv[]) {
    user_arguments_t user_params; 
    arguments_t params;
    
    while (1) {
        printf("PID=%ld>", (long) getpid());
        char val;
        val = getchar();
        
        if (val == '?') {
            print_help();
            val = getchar(); 
            continue;
        }

        ungetc(val, stdin); //gestion du \n
        val = getchar(); 

        if (val == '\n')
            continue;
        
        ungetc(val, stdin);
        int nb;
        nb = scanf("%c %c %i %i %c", &user_params.cmd, &user_params.l_type, &user_params.start, &user_params.length, &user_params.whence);
            
        switch(user_params.cmd) {
            case 'g':
                printf("cmd = F_GETLK \n");
                params.cmd = F_GETLK;
                break;
            case 's':
                printf("cmd = F_SETLK \n");
                params.cmd = F_SETLK;
                break;
            case 'w':
                printf("cmd = F_SETLKW \n");
                params.cmd = F_SETLKW;
                break;
            default:
                break;
        }

        switch(user_params.l_type) { 
            case 'w':
                printf("l_type = F_WRLCK \n");
                params.l_type = F_WRLCK; 
                break;  
            case 'r':
                printf("l_type = F_RDLCK \n");
                params.l_type = F_RDLCK;
                break;
            case 'u':
                printf("l_type = F_UNLCK \n");
                params.l_type = F_UNLCK;
            default:
                break;
        }

        switch(user_params.whence) {
            case 'c':
                params.whence = SEEK_CUR;
                break;
            case 'e':
                params.whence = SEEK_END;
                break;
            case '\n':
                params.whence = SEEK_SET;
                break;
            default:
                params.whence = SEEK_SET;
                break;
        }
        params.length = user_params.length;
        params.start = user_params.start;
        
    
        getchar(); //virer le fils de pute de \n.

        /*printf("parameters are : %c, %c, %i, %i, %c \n", user_params.cmd,
        user_params.l_type, user_params.start, user_params.length, user_params.whence);*/
        
        
        //ouverture du fichier. 
        int fd = open(argv[1], O_RDWR);
    
        struct flock f1; 
        f1.l_type = params.l_type;
        f1.l_whence = params.whence;
        f1.l_start = params.start;
        f1.l_len = params.length;

        //struct * flock permet de passer ou recevoir les paramètres du verrou.
        int status = fcntl(fd, params.cmd, &f1);
        printf("status = %i \n", status);
        
        //On récupère de l'information sur le verrou.
        if (params.cmd == F_GETLK) { /* F_GETLK*/
            //Si on a réussi à récupérer les infos du verrou.
            if (status == 0) {
                //fcntl retourne F_UNLCK si le verrou est placable.
                //si on aimerait mettre un verrou exclusif.
                if ((f1.l_type == F_UNLCK) && (params.l_type == F_WRLCK)) {
                    printf("[PID=%ld] can place exclusif lock (currently). \n", (long) getpid());
                //si on ne peut pas le mettre.
                } else if ((f1.l_type != F_UNLCK) && (params.l_type == F_WRLCK)) { 
                    printf("[PID=%ld] cannot place exclusif lock. (exclusif lock held by PID=%d) \n", (long) getpid(), f1.l_pid);
                }
                
                //si on aimerai mettre un verrou partagé
                if ((f1.l_type == F_UNLCK) && (params.l_type == F_RDLCK))  {
                    printf("[PID=%ld] Can place read lock \n", (long) getpid());
                } else if(params.l_type == F_RDLCK){ //si on ne peux pas le mettre.
                    printf("[PID=%ld] cannot place shared lock. \n", (long) getpid());
                }

                //si on aimerait enlever un verrou.
                if ((f1.l_type == F_UNLCK) && (params.l_type == F_UNLCK)) {
                    printf("[PID=%ld] Can remove lock. \n", (long) getpid());
                } else if(params.l_type == F_UNLCK) { //si on ne peut pas l'enlever.
                    printf("[PID=%ld] cannot remove lock. \n", (long) getpid());
                }
            } else if (errno == EAGAIN) {
                // process results and print informative text
                printf("Operation denied due to locks held by other processes. \n");
            }
        } else { /* F_SETLK, F_SETLKW */
            // check status and handle errors (look at manual for possible errors)
            if (status == 0) {
                
                switch(params.cmd) {
                    case F_SETLK:
                        //si on a placé un readlock.
                        if (params.l_type == F_RDLCK) {
                            printf("[PID=%ld] Set read lock without errors. \n", (long) getpid());
                            break;
                        }
                        if((params.l_type == F_WRLCK) && (f1.l_type == F_WRLCK)) {
                            printf("[PID=%ld] Set write lock without errors. \n", (long) getpid());
                            break;
                        }
                        if (params.l_type == F_UNLCK) {
                            printf("[PID=%ld] Released lock without errors. \n", (long) getpid());
                            break;
                        }
                        break;

                    case F_SETLKW:
                        //si on a placé un readlock.
                        if((params.l_type == F_RDLCK) && (f1.l_type == F_RDLCK)) {
                            printf("[PID=%ld] Set wait read lock without errors. \n", (long) getpid());
                            break;
                        }
                        if((params.l_type == F_WRLCK) && (f1.l_type == F_WRLCK)) {
                            printf("[PID=%ld] Set wait write lock without errors. \n", (long) getpid());
                            break;
                        }
                        if((params.l_type == F_UNLCK) && (f1.l_type == F_UNLCK)) {
                            printf("[PID=%ld] Released wait lock without errors. \n", (long) getpid());
                            break;
                        }
                        break;
                }                
            } else if ((errno == EAGAIN) || (errno == EACCES)) {
                //process results and print informative text
                printf("%s\n", strerror(errno));
                printf("Access error, another process is holding an exclusif or shared lock. \n");
            }
        }
    }
    return 0;
}