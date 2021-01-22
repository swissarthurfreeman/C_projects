#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define PATH_LOG "/var/log"
#define NAME_LOG "myownlog.log"

void exit_err (const char* str) {
    perror(str);
    exit( EXIT_FAILURE );
}

/* =========== getNbProc ===========
Cette fonction retourne le nombre de processus du systeme ,
vous ne serez pas interroge sur cette fonction
sortie: le nombre de processus actifs du systeme
note: la fonction termine le processus d’appel en cas d’erreur
=========== getNbProc ===========*/
int getNbProc(void) {
    struct sysinfo si;
    if(sysinfo(&si) != 0)
        exit_err("Cannot get system info");
    return si.procs;
}


void handler(int sig) {
    static FILE* logFile;
    int nbProc;
    nbProc = getNbProc();
    if(( logFile = fopen(NAME_LOG ,"a")) == NULL)
        exit_err("Cannot open log file");
    
    //on écrit dans le fichier le nb de processus.
    fprintf(logFile , "User call: %d\n", nbProc);
    fclose(logFile);
}

int main(void) {

    pid_t pid;
    int fd;
    static FILE* logFile;

    pid = fork(); //on crée un enfant
    //l'enfant hérite du masque et du session id du parent.
    //EID est celui du root, RID est celui de la personne qui lance.
    if(pid < 0)
        exit_err("Cannot fork");
    else if(pid != 0)
        return 0; //on ferme le parent. (l'enfant se ratache a init)

    //setsid crée un nouveau groupe de processus dont
    //l'appeleur qui n'est pas le leader deviendra le leader (e.g l'enfant)
    //e.g. son pid est le même que le pid du groupe
    //on fait ça afin que quand la session de l'utilisateur se ferme (logout),
    //on ne ferme pas le processus.
    if(setsid() == -1)
        exit_err("Cannot setsid");
    
    //on change la racine pour le processus
    //en PATH_LOG (e.g /var/log)
    //il faut être root afin de pouvoir le faire, c'est pour cela qu'on a fait
    //chmod u+s.
    if(chroot(PATH_LOG) != 0)
        exit_err("Cannot change root");
    
    // "/" ici après chroot sera /var/log
    //il faut aussi changer le répertoire courant.
    if(chdir("/") != 0)
        exit_err("Cannot change path");

    //umask vas changer le masque de permission de l'enfant a
    //(S_IXUSR | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH) & 0777
    //e.g. umask ENLEVE les permissions complémentaires de (S_IXUSR | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH)
    //donc on aura que --x--xrwx
    //l'enfant ici a toujours un EUID de root ! 
    //umask sont les permissions A ENLEVER lorsque l'on crée un fichier.
    umask(S_IXUSR | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);

    //on ouvre un stream NAME_LOG "myownlog.log"
    //myownlog.log sera a la racine du processus (e.g. /var/log)
    //quand on crée un fichier, le masque du fichier crée sera 0666 & ~MASQUE_PROCESSUS
    //on aura rw-rw---
    if(( logFile = fopen(NAME_LOG, "w")) == NULL)
        exit_err("Cannot open log file");
    fclose(logFile);

    //getuid récupère le real user id (e.g. celui de l'utilisateur qui a lancé le processus)
    //getgid retourne le real group id du groupe du processus (e.g. le groupe de l'utilisateur qui a lancé le processus)
    //chown change le propriétaire et le groupe du fichier NAME_LOG.
    //maintenant le fichier appartient a l'utilisateur qui a lancé et a son groupe.
    chown(NAME_LOG , getuid(), getgid());

    //setuid set le EFFECTIVE USER ID 
    //getuid renvoie le REAL USER ID
    //donc on set l'effectif user  ID (qui est root ici) au réel (e.g. on perds les droits root)
    //et on a plus que les droits utilisateur. (on peut plus chroot)
    setuid(getuid());

    //On rouvre le fichier NAME_LOG qui appartient a l'utilisateur et dont le groupe est celui de l'utilisateur.
    //ce fichier a les premissions : 
    if((fd = open(NAME_LOG , O_WRONLY | O_APPEND)) == -1)
        exit_err("Cannot open log file for input / output redirection");
    
    //On redirige la sortie standard d'erreur et la sortie standard vers ce fichier,
    if( (dup2(fd , STDERR_FILENO ) == -1) || (dup2(fd , STDOUT_FILENO ) == -1) )
        exit_err("Cannot exchange file descriptors");

    close(fd);

    
    struct sigaction sa;
    sa.sa_handler = handler; //handler définit plus haut.
    
    sigfillset (&sa.sa_mask);
    
    sa.sa_flags = 0;

    //on écoute le signal SIGUSR1.
    //a chaque signal on écrit dans le fichier /var/log/NAME_LOG
    sigaction(SIGUSR1 , &sa , NULL);

    while(1)
        pause();

    return 0;
}

/*
Questions :

On fait une "prison chroot" : 

1. Que fait le programme suivant ?

Il crée un démon qui écrit dans un 


2. D’après la structure du programme et ces appels aux fonctions système, comment
appel-t-on ce type de programme ?

C'est un prison chroot

3. A quoi sert l’entrée "install" du makefile ?



4. Décrivez le code et son fonctionement en détail.



5. Quel sera l’utilisateur, le groupe et les droits d’accès du fichier de log une fois
celui-ci créé.



6. Quelle est la commande à taper depuis un terminal pour activer le handler du
programme ?

KILL -s SIGUSR1 PID

*/