#include "message.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * Send a string on a pipe
 */
int send_string(int fd, const char *str){
    // Check parameters
    if(fd < 0){
        fprintf(stderr,"send_string : Invalid file descriptor\n");
        return -1;
    }
    if(str == NULL){
        fprintf(stderr,"send_string : the string is NULL\n");
        return -1;
    }

    // Get the string size
    size_t size = strlen(str);

    // Write the string size on the pipe
    if(write(fd,&size,sizeof(size_t)) == -1){
        perror("write");
        return -1;
    }

    // Write the string one the pipe
    if(write(fd,str,size*sizeof(char)) == -1){
        perror("write");
        return -1;
    }

    return 0;
}

/**
 * Receive a string from a pipe
 */
char *recv_string(int fd){
    // Check parameters
    if(fd < 0){
        fprintf(stderr,"recv_string : Invalid file descriptor\n");
        return NULL;
    }

    // Read the string size on the pipe
    size_t size = -1;
    if(read(fd,&size,sizeof(size_t)) == -1){
        perror("read");
        return NULL;
    }
    if(size <= 0){
        fprintf(stderr,"recv_string : no string in the pipe");
        return NULL;
    }

    
    char *str = calloc(size+1,sizeof(char));
    if(str == NULL){
        perror("calloc");
        return NULL;
    }

    // Read the string on the size
    if(read(fd,str,size*sizeof(char)) == -1){
        perror("read");
        return NULL;
    }
    
    return str;

}





int send_argv(int fd, char *argv[]){
    printf("c\n");
    return 0;
}

char **recv_argv(int fd){
    printf("d\n");
    return NULL;
}