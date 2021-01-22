CC = gcc
CFLAGS = -g -Wall --pedantic

all: initialize.out operations.out

initialize .out: initialize.o
	$(CC) $(CFLAGS) initialize.o -o initialize.out


operations .out: operations.o
	$(CC) $(CFLAGS) operations.o -o operations.out

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *.dat *.out