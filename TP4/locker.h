extern void print_help();

typedef struct {
    __u_int cmd;
    __u_int l_type;
    __u_int start;
    __u_int length;
    __u_int whence;
} arguments_t;

typedef struct {
    char cmd;
    char l_type;
    __u_int start;
    __u_int length;
    char whence;
} user_arguments_t;