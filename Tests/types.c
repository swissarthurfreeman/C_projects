#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int main() { 
    /*unsigned int a;
    a = -1;
    printf("%u\n", a);*/
/*
    bool myBool = true;
    printf("%d \n", myBool);*/
    /*
    printf("%d \n", CHAR_MIN); //affiche -128
    printf("%d \n", UINT_MAX); //affiche -1
    printf("%u \n", UINT_MAX); //affiche 2^32 - 1
    printf("%lu \n", __UINT_FAST64_MAX__); //affiche 2^64 - 1*/

    /*
    newString = strncpy(newString, mot, strlen(mot) + 1);
    newString[0] = 'B';
    printf("%s\n", newString); //affiche Bello
    printf("%s\n", mot); //affiche hello*/
    /*char mot[] = "Hello";
    char * newString; 
    newString = malloc(sizeof(char)*strlen(mot));
    newString = mot;
    mot[0] = 'A';
    printf("%s\n", newString); //Affiche Aello
    printf("%s\n", mot); //Affiche Aello*/

    /*char * p;
    int n;
    n = scanf("%m[a-z, space]", &p);
    printf("read %s", p);
    free(p);
    return 0;*/

    char * pointeur; //stocké dans la pile à l’adresse 0x4552
    pointeur = malloc(sizeof(char)*4);
    pointeur = "Hal";
    printf("%s\n", pointeur); //affiche Hal
    int * value;
    value = malloc(sizeof(int));
    *value = 10;
    printf("%d\n", *value);
}

//#include <stdio.h>
/* La fonction main est toujours la première fonction du programme à être
appellée lors que l'execution*/
/*int main(void)
{
  float inputFloat;
  char inputChar;
  int nbCorrespondance;

  //Entrée de l'utilisateur pour un float et affichage du
  //float de différente manières
  printf("Veuiller entrer un float:\t");
  scanf("%f", &inputFloat);
  printf("Affichage de l'entree sous forme de float: %f\n", inputFloat);
  printf("Idem avec 2 digits apres la virgule: %.2f\n", inputFloat);
  printf("Idem avec au moins 6 caractères: %6.2f\n", inputFloat);
  printf("Notation scientifique: %e\n", inputFloat);
  

  //Boucle tant que l'utilisateur n'appuye pas sur 'enter' uniquement
  do
  {
    //Entrée de l'utilisateur pour un caractère
    //avec effacement du buffer d'entrée clavier
    printf("Veuiller entrer un caractere: ");

    while((inputChar = getchar()) != '\n' && inputChar != EOF);
    inputChar = getchar(); //could also use scanf("%c", &inputChar);

    //Si le caractère est valide l'afficher sous forme d'entier non signé
    if(inputChar != '\n')
      printf("Le code ASCII de %c est %u\n", inputChar, inputChar);
  } while(inputChar != '\n');
  return 0;
} //end main*/