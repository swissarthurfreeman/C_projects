#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types-h>
#include <sys/wait b>
#include <string.h>
#include <errno.h>

//macros sigusr1,2 pour usage par l'utilisateur.
//c'est ce qu'on fait ici.
#define SIG_CAFE SIGUSR1
#define SIG_CREME SIGUSR2
#define SIG_ANUL SIGINT

const char * const cremeStr = "Ajout de creme sur le cafe\n";
const char * const anulStr = "Cafe interrompu par l'utilisateur\n";
const char * const errStr = "Erreur survenue, cafe interrompu\n";

pid_t pidService = 0;

void exit_err(const char* str) {
    perror(str);
    exit(EXIT_FAILURE);
}

//fonction qui lie signum au handler sig_handler, sans options additionnelles.
void set_sigaction_handler(int signum, void (*sig_handler)(int)) {
    /*configurer des handlers sur des signaux fonctionne toujours selon le
    schéma suivant :
    1) On définit une struct sigaction
    2) On la configure
    3) On effectue un sigaction qui lie cette struct et ses propriétés
    au signal que l'on veux gérer. 
    NB: Il existe l'appel système sigaction et la structure sigaction.
    man sigaction.
    */
    struct sigaction action;

    if(sigemptyset(&action.sa_mask) == -1)
      exit_err("set_sigaction_handler, sigemptyset")

    //pas de SA_SIGINFO, donc on utilise le handler normal
    //(pas de structure info en plus, seulement le numéro du signal reçu)
    action.sa_flags = 0; //on ne spécifie pas de comportements spéciaux.
    action.sa_handler = sig_handler; //on associe le handler au signum.
    if(sigaction(signum, &action, NULL) == -1) //on ne conserve pas l'ancienne struct.
      exit_err("set_sigaction_handler, sigaction")
}

//handler de service de l'enfant.
void service_handler(int sig) {
    switch(sig) {
        case SIG_CREME:
          write(STDIN_FILENO, cremeStr, strlen(cremeStr));
          break;
        case SIG_ANUL:
          write(STDIN_FILENO, anulStr, strlen(anulStr));
          exit(EXIT_SUCCESS);
          break;
        default:
          fprintf(stderr, "Signal %d non attendu\n", sig);
          exit(EXIT_FAILURE);
    }
}

void service() {
    //l'enfant doit accepter ces deux signaux.
    set_sigaction_handler(SIG_CREME, service_handler);
    set_sigaction_handler(SIG_ANUL, service_handler);

    int err;
    sigset_t set, oldset;

    //ensemble de signaux vide.
    err = sigemptyset(&set);

    //Ou logique bit par bit.
    //sigaddset retourne 0 en cas de succès, -1 en cas d'erreur.
    err |= sigaddset(&set, SIG_CREME);

    //err sera 0 uniquement si les deux valeurs de retour
    //de sigemptyset/addset valent 0 donc y'a pas eu d'erreur
    if(err != 0)
      exit_err("service, construction d'ensemble de signaux");
    
    //sigprocmask modifie l'ensemble des signaux bloqués.
    //SIG_BLOCK fait l'union du set actuel et de l'ancien,
    //au passage on récupère l'ancien set avec oldset (ou on bloquait pas creme)
    //e.g on se mets a bloquer SIG_CREME.
    if(sigprocmask(SIG_BLOCK, &set, &oldset) == -1) 
      exit_err("service, sigprocmask 1");

    printf("Je commence a preparer un cafe\n");
    sleep(5); //on peut recevoir les signaux anul en attendant.

    //une fois fait, on remets le maske a oldset (ou on bloquait pas creme)
    if(sigprocmask(SIG_SETMASK, &oldset, NULL) == -1)
      exit_err("service, sigprocmask 2");
    
    printf("Cafe termine\n");
    exit(EXIT_SUCCESS);
}

void btn_handler(int sig) {
    switch(sig) {
        //si on nous demande de faire un cafe.
        case SIG_CAFE :
            pidService = fork();  
            if( pidService == -1) //gestion erreur de fork.
                exit_err("btn_handler , fork");
            //dans le parent (pid > 0)
            else if ( pidService > 0) {
                //on s'occupe de creme et d'annuler, maintenant que la commande
                //est passée.
                set_sigaction_handler(SIG_CREME , btn_handler);
                set_sigaction_handler(SIG_ANUL , btn_handler);

                //on attends que notre enfant finisse le service.
                int status = 0;
                //on attends jusqu'à ce que l'enfant change d'état, soit on reçoit SIGCHLD
                //soit on reçoit un signal non bloqué qui interromp un appel système, enquel
                //cas EINTR sera égal a ERRNO.
                //Ici y'a une erreur, status devrait être le deuxième paramètre de waitpid.
                while( (status = waitpid(pidService , NULL , 0)) == -1)
                    //EINTR est reçu lorsqu'un signal est reçu
                    if(errno != EINTR)
                        exit_err("btn_handler , waitpid");
                //WIFEXITED vérifie que l'enfant s'est bien terminé.
                if( WIFEXITED(status))
                    //si on a pas retourné 0 depuis le main (e.g y'a eu un truc bizzare)
                    if( WEXITSTATUS(status) != 0)
                        write(STDIN_FILENO , errStr , strlen(errStr)); //errStr globale
            } else {
                //dans l'enfant on effectue le service.
                service();
            }
            break;

        //SIG_CREME recu
        case SIG_CREME :
            //on dit a l'enfant de mettre de la creme.
            if(kill(pidService , SIG_CREME) == 1)
                exit_err("btn_handler , kill");
            break;

        //SIG_ANUL recu
        case SIG_ANUL :
            //on dit a l'enfant de se terminer.
            if(kill(pidService , SIG_ANUL) == 1)
                exit_err("btn_handler , kill");
            break;

        default:
            fprintf(stderr , "Le signal %d non attendu\n", sig);
            exit( EXIT_FAILURE );
    }
}

int main(void) {
    set_sigaction_handler(SIG_CAFE , btn_handler);

    //SIG_IGN est un "handler" qui vas ignorer ces signaux.
    //on veux que les gérer si on produit un café !
    set_sigaction_handler(SIG_CREME , SIG_IGN);
    set_sigaction_handler(SIG_ANUL , SIG_IGN);

    while(1)
        //pause attends un signal (man pause).
        pause();

    return 0;
}

/* Questions :

1. Combien de processus ce programme génère-t-il ? Décrire le rôle et les actions
de chaque processus.

Ce processus vas générer un seul processus, le parent, qui vas attendre des signaux.
Lors de la réception du signal SIG_CAFE, on vas forker et créer un enfant, qui vas
"produire" un café. (REGARDER LE CODE ET COMMENTER EN DIRECT SUR LE PDF)

2. Que se passe-t-il si le bouton CAFE est pressé plusieurs fois de suite ?

Par défaut en C lorsqu'un handler s'éxécute pour un signal, et qu'on reçoit un signal
qui invoque le même handler, ce signal sera masqué jsuqu'à ce que le handler se termine.
Si on appuie plusieurs fois de suite le même bouton pendant que le premier café se
produit, un deuxième café se produira et ce sera tout.

3. Que se passe-t-il si l’on appuye sur SIG_CREME sans qu’un café ne soit lancé ?
Que se passe-t-il si l’on appuye sur SIG_CREME lorsqu’un café est lancé ?

Il ne se passe rien dans le premier cas, en effet, au signal SIG_CREME nous avons
associé dans le main SIG_IGN qui est une fonction qui ignore ce signal. On aurait 
pû le faire via un masque aussi, en utilisant sigprocmask.

Dans le deuxième cas, on change la donne. En effet, si on est en train de produire un café
le masque du parent a changé, et on réceptionne les signaux SIG_CREME et SIG_ANUL.
Ce qui vas se passer c'est que dans btn_handler, si on a déjà lancé un café, on a un enfant qui
produit, et on vas executé le case SIG_CREME du btn_handler (car on l'aura associé en faisant le fork)
et vu que pidService est globale, on pourra rediriger SIG_CREME vers l'enfant.

4. Dans quel cas le programme affiche-t-il l’erreur "Erreure survenue, cafe interrompu" ?

Avec votre code, vous utilisez le macro IFEXITED(status) excepté que status ici est le pid
de l'enfant, on ne vas absolument pas avoir le comportement souhaité on ne sait pas comment
se comportera WEXITSTATUS avec des valeurs qui ne sont pas des status. Si vous l'aviez implémenté
correctement, on aurait ce message uniquement lorsque l'enfant est interrompu sauvagement, e.g.
retourne quelque chose différent de 0 depuis son main.

5. Pourquoi utiliser la fonction "write" au lieu de "printf" dans les handlers de
signaux ?

Cela fait partie des règles de programmation des handlers, on évite les effets de bord.


*/
