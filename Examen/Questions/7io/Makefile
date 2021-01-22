CC=gcc
CFLAGS=-Wall -g --pedantic

all: writeVec.out readVec.out

writeVec.out: writeVec.o
	$(CC) $(CFLAGS) -o writeVec.out writeVec.o

writeVec.o: writeVec.c
	$(CC) $(CFLAGS) -c -o writeVec.o writeVec.c

readVec.out: readVec.o
	$(CC) $(CFLAGS) -o readVec.out readVec.o

readVec.o: readVec.c
	$(CC) $(CFLAGS) -c -o readVec.o readVec.c

clean:
	rm -rf *.o *. out *. dat