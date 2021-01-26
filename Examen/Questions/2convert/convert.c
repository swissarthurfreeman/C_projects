#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//remplacé dans la précompilation,
//le nom n'apparaît jamais dans le code.
#define E_NOT_VALID_CHAR -1
#define NB_BITS_OCTETS 8

//short est sur 16 bits. (2 octets)
//typedef unsigned long long int number;
typedef short int number; 
//typedef __uint128_t number;

char errorMsg [] = "Erreur non documentee\n"; //Data Segment
int unUsedVariable; //BSS Segment

//chaine est le param fourni par l'utilisateur input est un pointeur
//sur number (définit dans le main)
//cette fonction convertit la chaine qui représente un nombre 
//en un nombre int.
int convert(number* res, const char* chaine) { //Data Segment
    //res et chaine sont mises dans une nouvelle frame de la pile.
    //chaine est un tableau d'entiers !
    int i, length = 0; //Pile

    //initialise res en tant que nul.
    *res = 0;

    if(*chaine == '\0') //si la chaîne est vide.
        return E_NOT_VALID_CHAR ;

    while(chaine[length] != '\0') //on compte le nb de caractères dans la chaine.
        length ++; //(chaine est argv[1] et argv un tableau de strings)

    length = length - 1; //on ne compte pas '\0'

    //depuis le dernier caractère (différent de 0) jusqu'au début
    for(i=length; i >= 0; i--) {
        //si on trouve un nb < 0 ou un nb > 9 c'est que c'est pas
        //des entiers (table ASCII [48..57] = [0..9]) on gère pas les nombre signés.
        if( (chaine[i] < '0') || (chaine[i] > '9') )
            return E_NOT_VALID_CHAR; //c'est pas valide.

        //si c'est valide, on convertit en entier en base 10
        //chaine[i] - '0' renvoie la valeur entière représentée par
        //le caractère, exemple avec 967874 (en bas)
        //on fait * pow(10, length-i) afin de lui donner le poids adéquat
        
        *res += (chaine[i] - '0') * pow(10, length - i);
        printf("%d\n", (short int) HUGE_VAL);
        //printf("%d %i \n", *res, length - i);
    }
    //number test;
    //test = *res;
    //printf("%d\n", test);
    //printf("%ld\n", sizeof(test));
    printf("%d \n", *res);
    return 0;
}

//cette fonction prends un nombre value et renvoie
//le string de sa représentation binaire.
char * getStr(number value) {
    number testBit; //Pile
    
    int i, nbBits; //Pile

    //on récupère le nb de bits de value 
    //(value est le résultat convert sur argv[1] !)
    nbBits = sizeof(value) * NB_BITS_OCTETS;

    //str est dans la pile mais *str est dans le tas 
    char* str = (char *) malloc(nbBits);
    
    if(str == NULL)
        return NULL;
    
    for(i = 0; i < nbBits; i++) {
        testBit = pow(2, i);
        if( (testBit & value) == testBit )
            str[nbBits - i - 1] = '1';
        else
            str[nbBits - i - 1] = '0';
    }
    //eh oh il oublie le '\0' a la fin.
    return str;
}

int main(int argc , char* argv[]) {
    //pile
    number input; //typedef short int number.
    //str est dans la pile *str dans le tas
    char* str;
    if(argc < 2) {
        fprintf(stderr , "Usage: convert entier\n\n");    
        return -1;
    }

    //convert change l'input a sa valeur convertie via convert sur argv[1].
    if(convert(&input , argv[1]) != 0) {
        fprintf(stderr , "%s", errorMsg); //vous donnez la même erreur de message... hmmm...
        return -1;
    }

    if( (str = getStr(input)) == NULL) {
        fprintf(stderr , "%s", errorMsg);
        return -1;
    }
    printf("%s\n", str);
    free(str);

    return 0;
}

/*
Questions :

1. Que fait le programme suivant ?

Le programme prends un nombre fournit par l'utilisateur, excepté que argv est un tableau
de chaines de caractères, donc il faut convertir en un entier, puis le programme 
renvoie la représentation binaire de cet entier.

2. A l’aide d’un schéma, représentez l’espace d’adressage virtuel du processus sous linux
et ses différents segments.

cf.schéma en local

3. Décrivez le code et son fonctionement en détail. Au fur et a mesure de vos explications indiquez 
pour chaque variable dans quelle partie de l’espace d’adressage elle se situe. Indiquez également 
lorsque le tas est modifié.


4. La chaine de caractère str est mal construite. Que lui manque-t-il ?

'\0'

5. Que se passe-t-il si j’appel le programme de la manière suivante:
convert 546213354863513268423132654
Comment éviter ce problème ?

Il faut 89 bits afin de représenter ce nombre, on peut utiliser le type __uint128_t.

Débordement, c'est un nombre qui n'est pas exmprimable sur 2 octets.
En fait avec 546213354863513268423132654 ce sera 00000000000000, car pow renvoie
l'erreur HUGE_VAL, le comportement devient indéfini.

Exemple de convert avec : 
i =        0    1    2    3    4    5
chaine = ['9', '6', '7', '8', '7', '4']
si i = 4 on a 
chaine[i] = '7'
'7' == 55 (table ascii) 
'7' - '0' = 55 - 48 = 7 
Et 7 * 10^(5 - 4) = 70 ce qui est bien le poids
de la valeur 7 en position 4.
*/