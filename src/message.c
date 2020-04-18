#include "message.h"
#include <stdio.h>

int send_sring(int fd, const char *str){
    printf("a\n");
    return 0;
}

char *recv_string(int fd){
    printf("b\n");
    return NULL;
}

int send_argv(int fd, char *argv[]){
    printf("c\n");
    return 0;
}

char **recv_argv(int fd){
    printf("d\n");
    return NULL;
}