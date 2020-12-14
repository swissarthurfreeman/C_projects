#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#define SIZE 20000000
#define PLEASE_WAIT -1

typedef unsigned int number_t;

void OnError(const char *str) {
	perror(str);
	exit( EXIT_FAILURE );
}


int prod( number_t *data) {
	int fd;
	if( (fd = open("/dev/urandom", O_RDONLY)) == -1)
		OnError("open 2");
	for(int i=0;i<SIZE;i++)
		if(read(fd , data+i, sizeof(number_t)) != sizeof(number_t))
			OnError("read");
	close(fd);
}

int cons( number_t *data) {
for(int i=0;i<SIZE;i++) {
	while(*( data+i) == PLEASE_WAIT )
		sched_yield ();
	printf("%d: %x\n", i, *( data+i));
	}
}

int main(int argc , char* argv []) {
	int fd , i;
	number_t *data;
	if(argc != 2) {
		printf("Usage: one filename\n\n");
		exit (0);
	}

	size_t mem_size = sizeof( number_t)*SIZE;
	if( (fd = open(argv [1], O_RDWR | O_CREAT , S_IRUSR | S_IWUSR)) == -1)
		OnError("open");

	if( ftruncate (fd , mem_size) == -1)
		OnError("ftruncate");

	data = mmap(NULL , mem_size , PROT_READ | PROT_WRITE , MAP_SHARED , fd , 0);
	
	if(data == MAP_FAILED )
		OnError("mmap");

	for(int i=0;i<SIZE;i++)
		*( data+i) = PLEASE_WAIT ;

	pid_t res = fork();
	if(res == 0)
		cons(data);
	else if(res > 0)
		prod(data);
	else
		OnError("fork");

    if(munmap(data , mem_size ) == -1)
    	OnError("munmap");
    
    close(fd);
	return 0;
}