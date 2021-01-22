#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

int main(int argc , char* argv []) {
    
    struct rlimit limites;
    getrlimit(RLIMIT_NPROC, &limites);
    printf("%ld\n", limites.rlim_cur);
    
    limites.rlim_cur = 500;
    limites.rlim_max =500;
    setrlimit(RLIMIT_NPROC, &limites);

    getrlimit(RLIMIT_NPROC, &limites);
    printf("%ld\n", limites.rlim_cur);
    //exit(EXIT_SUCCESS);

    pid_t pid = 0;
    int status;
    int cont = 1;
    //pid == 0 dans l'enfant
    //dans le parent le pid est celui de l'enfant.
    while(pid == 0) {
        pid = fork ();
        if (pid < 0) {
            perror("fork");
            //access return value with $? EAGAIN === 11
            //printf("fork");
            //cont = 0;
            //break;
            exit(errno);
        }
    }

    //if(cont) 
    waitpid(pid , &status , 0);

    printf("%d\n", status);
    //WIFEXITED renvoie vrai si l'enfant c'est bien terminé normalement.
    if WIFEXITED (status) {
        //on imprime la valeur retournée par l'enfant
        //dans le dernier enfant, quand on dépasse la limite de ressources
        //on renvoie EACCESS
        printf("status: %d\n", WEXITSTATUS (status));
    }
    return errno;
}
/*
1. Que fait le programme suivant ?

Ce programme crée un enfant, qui crée un enfant, qui crée un enfant, tant que 
c'est possible jusqu'à une erreur générée par fork (très certainement une limite
de mémoire, mais y'a d'autres erreurs possibles (interruption par signal)).

Processus
1              2            n-1         n
fork_1 --> fork_2 --> ... fork_n-1 --> fork_n --> error EAGAIN
waitpid_1 <-- waitpid_2 <-- ... waitpid_n-1

2. Quel est l’impact de ce programme sur les ressources de l’ordinateur, notamment
en matière de mémoire vive ?

fork() effectue une copie du processus courant en mémoire, forker a l'infini
finira par totalement remplir la mémoire a l'infini si les enfants ne se terminent
pas. ça la remplit entièrement, tant que c'est possible. Il est possible d'éviter
cela en appliquant des limites sur le processus via setrlimit, avec le flag RLIMIT_NPROC
on peut limiter la quantité de processus engendrée par ce processus, et si la soft limit 
est dépassée fork() génère une erreur EAGAIN.

3. A la ligne 16, est-ce que le code d’erreur errno est nécéssairement le même que le
code d’erreur rapporté par perror ?

Oui, perror prends un string de l'utilisateur qui décrit (en général) la fonction
qui a généré une erreur, et perror accède a errno qui est une variable globale, 
et ira chercher dans la liste des erreurs globales du système, qui est indexée par
errno. (du moins a l'époque selon la man)

4. A la ligne 22, a quoi correspond la valeur de WEXITSTATUS(status) par rapport
au processus enfant (i.e. à quelle autre variable est-elle reliée) ?

Elle correspond a la valeur retournée par l'enfant, e.g. errno, car si on exit
on le fait avec errno, et si on retourne on retourne avec errno. 
WEXITSTATUS renvoie le exit status de l'enfant, c'est
le paramètre fourni a exit, ou retourné dans le main.

5. Ce programme peut-il créer des zombies ? Pourquoi ?

Non, chaque enfant attends bien sur sa descendance récursivement,
on aura pas de zombies.

*/