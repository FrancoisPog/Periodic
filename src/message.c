#include "message.h"
#include "perror.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * Send a string in a pipe
 */
int send_string(int fd, const char *str){
    // Check parameters
    if(fd < 0){
        fprintf(stderr,"> Error : [send_string] - Invalid file descriptor\n");
        return -1;
    }
    if(str == NULL){
        fprintf(stderr,"> Error : [send_string] - The specified string is NULL\n");
        return -1;
    }

    // Get the string size
    size_t size = strlen(str);

    // Write the size in the pipe
    write_perror(fd,&size,sizeof(size_t));

    // Write the string in the pipe
    write_perror(fd,str,size*sizeof(char));

    return 0;
}

/**
 * Receive a string from a pipe
 */
char *recv_string(int fd){
    // Check parameters
    if(fd < 0){
        fprintf(stderr,"> Error : [recv_string] - Invalid file descriptor\n");
        return NULL;
    }

    // Read the size in the pipe
    size_t size;
    ssize_t n = read_perror(fd,&size,sizeof(size_t));
    if(n == 0){
        fprintf(stderr,"> Error : [recv_sting] - Nothing to read in the pipe\n");
        return NULL;
    }

    char *str = calloc_perror(size+1,sizeof(char));
    
    // Read the string in the pipe
    read_perror(fd,str,size*sizeof(char));
    
    return str;
}



/**
 * Send an array of string in a pipe
 */
int send_argv(int fd, char *argv[]){
    // Check parameters
    if(fd < 0){
        fprintf(stderr,"> Error : [send_argv] - Invalid file descriptor\n");
        return -1;
    }
    if(argv == NULL){
        fprintf(stderr,"> Error : [send_argv] - The specified array is NULL\n");
        return -1;
    }

    // Get the argv length
    size_t size = 0;
    while(argv[size++] != NULL){}
    size--;

    if(size == 0){
        fprintf(stderr,"> Warning : [send_argv] - The array is empty\n");
    }
   
    // Write the array size in the pipe
    write_perror(fd,&size,sizeof(size_t));

    // Send each string in the pipe
    for(size_t i = 0 ; i < size ; ++i){
        if(send_string(fd,argv[i]) == -1){
            return -1;
        }
    }
    

    return 0;
}


/**
 * Receive an array of string from a pipe
 */
char **recv_argv(int fd){
    // Check parameters
    if(fd < 0){
        fprintf(stderr,"> Error : [recv_argv] - Invalid file descriptor\n");
        return NULL;
    }

    // Read the array size
    size_t size;
    ssize_t n = read_perror(fd,&size,sizeof(size_t));
    if(n == 0){
        fprintf(stderr,"> Error : [recv_argv] - Nothing to read in the pipe\n");
        return NULL;
    }

    if(size == 0){
        fprintf(stderr,"> Warning : [recv_argv] - The receive array is empty\n");
    }
    
    // Read each string in the pipe
    char **argv = calloc_perror(size+1,sizeof(char*));
    
    for(size_t i = 0 ; i < size ; ++i){
        argv[i] = recv_string(fd);
        if(argv[i] == NULL){
            free(argv);
            return NULL;
        }
    }
    argv[size] = NULL;

    return argv;
}