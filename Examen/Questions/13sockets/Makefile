CC=gcc
CFLAGS=-Wall -g --pedantic

all: client server

client: client.o
	$(CC) $(CFLAGS) -o client client.o

client.o: client.c common.h
	$(CC) $(CFLAGS) -c -o client.o client.c

server: server.o
	$(CC) $(CFLAGS) -o server server.o

server.o: server.c common.h
	$(CC) $(CFLAGS) -c -o server.o server.c

clean:
	rm -rf *o client server