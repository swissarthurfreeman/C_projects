/*getopt est une commande de C ! Mais aussi de Bash, il faut 
faire man 3 getopt pour lire la documentation de C sur le sujet !*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//To Do : -f les paramètres d'entrée doivent être
//considérés comme fichiers.
//sinon c'est une unique chaîne de caractères
//-t choisit le digest -t DIGEST

//arguments count and arguments values.
int main(int argc, char *argv[]) {
    printf("The number of arguments is : %d (this includes ./a.out) \n", argc);
    /*for(int i = 0; i < argc; i++) { //we use argc to navigate the argv array les mots entiers y sont.
        printf("%s \n", argv[i]);
    }*/
    char opt; 
    //getopt récupère les caractères suivant un tiré - 
    //un par un à chaque appel de getopt.
    opt = getopt(argc, argv, "if:lrx");
    printf("\n opt = %c \n", opt);

    opt = getopt(argc, argv, "if:lrx");
    printf("\n opt = %c \n", opt);
    if (opt == 108) { //on peut comparer les codes ASCII.
        printf("option is l \n");
    }
    //du moment qu'on a encore des options.
    
    return 0;
}