#include <stdlib.h>
#include <stdio.h>
#include "locker.h"

void print_help() {
    printf("\n");
    printf("    Format : cmd l_type start length [ whence ( optional ) ]\n");
    printf("    'cmd ' --- 'g ' ( F_GETLK ) , 's ' ( F_SETLK ) , or 'w ' ( F_SETLKW )\n"); 
    printf("    'l_type ' --- 'r ' ( F_RDLCK ) , 'w ' ( F_WRLCK ) , or 'u ' ( F_UNLCK )\n");
    printf("    'start ' --- lock starting offset\n"); 
    printf("    'length ' --- number of bytes to lock\n");
    printf("    'whence ' --- 's' ( SEEK_SET , default ) , 'c' ( SEEK_CUR ) , or 'e' ( SEEK_END ) \n");
    printf("\n");
}