/************************************************
*  Interface d'options                          *
*  Auteur : Arthur Freeman, Date : 11/10/2020.  *
*************************************************/ 

struct arguments {
    int t; //boolens indiquant si t ou f ont étés fournis. (0 ou 1)
    int f;
    char* digest; //pointeur sur le digest.
    char* message; //pointeur alloué si un message est fourni.
    int optind; //position des paramètres suivant -f.
};

//get_options parse et renvoie les options dans struct arguments
//au main, cachant la complexité de lire le standard input sur le shell.
extern struct arguments get_options(int argc, char *argv[]);