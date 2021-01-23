CC = gcc
#-Wall active tous les différents flags de warnings.
#pedantic active encore plus de différents warnings.
#-g done de l'information de débuggage propre au système.
#https://www.gnu.org/software/make/
CFLAGS = -g -Wall --pedantic

all: initialize.out operations.out

initialize.out: initialize.o
	$(CC) $(CFLAGS) initialize.o -o initialize.out


operations.out: operations.o
	$(CC) $(CFLAGS) operations.o -o operations.out

#le flag -c génère les fichiers .o
#% est un pattern qui fait du matching sur les noms
#de fichiers fournit en paramètres lors du make.
#e.g. make operations, idem sur .c
%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *.dat *.out