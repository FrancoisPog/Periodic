#define _POSIX_C_SOURCE 1
#include "file.h"
#include "perror.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>



int write_pid(){
    // Get the pid
    pid_t pid = getpid();

    // Check is period.pid already exists
    FILE *file = fopen("/tmp/period.pid","r");
    if(file != NULL){
        fprintf(stderr,"> Error [write_pid] - period is already running\n");
        fclose_perror(file);
        return -1;
    }
    
    if(errno != ENOENT){
        perror("fopen");
        return -1;
    }

    // Write the pid inside
    file = fopen_perror("/tmp/period.pid","w");
    
    fprintf(file,"%i",pid);

    fclose_perror(file);

    return pid;
}


int make_pipe(){
    if(mkfifo("/tmp/period.fifo",0777) == -1){
        if(errno !=  EEXIST){
            perror("mkfifo");
            return -1;
        }
        return 1;  
    }
    return 0;
}


int make_dir(){
    if(mkdir("/tmp/period",0777) == -1){
        if(errno != EEXIST){
            perror("mkdir");
            return -1;
        }
        return 1;
    }
    return 0;
}



int period_redirection(){
    int err = open_perror("/tmp/period.err",O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    dup2_perror(err,STDERR_FILENO);
    close_perror(err);
     
    int out = open_perror("/tmp/period.out",O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    dup2_perror(out,STDOUT_FILENO);
    close_perror(out);

    return 0;
}

/**
 * Redirects the standards file descriptor
 */ 
int command_redirection(char type,size_t cmdId){
    if(type != 'i' && type != 'o' && type != 'e'){
        return -1;
    }

    int fd = -1;
    char buf[32];
    switch(type){
        case 'i' : {
            sprintf(buf,"/dev/null");
            fd = STDIN_FILENO;
            break;
        }
        case 'o' : {
            sprintf(buf,"/tmp/period/%zd.out",cmdId);
            fd = STDOUT_FILENO;
            break;
        }
        case 'e' : {
            sprintf(buf,"/tmp/period/%zd.err",cmdId);
            fd = STDERR_FILENO;
            break;
        }
        default : {
            assert(0);
        }
    }
    assert(fd != -1);

    int out = open_perror(buf,O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    dup2_perror(out, fd);
    close_perror(out);

    return 0;
}


int get_period_pid(){
    FILE *file = fopen("/tmp/period.pid","r");
    // Test if the file exists or if an error has occured
    if(file == NULL){
        if(errno == ENOENT){
            return 0;
        }
        perror("open");
        return -1;
    }

    // Read the pid
    char buf[8] = "";
    int n = fread(buf,sizeof(char),8,file);

    if(!n){ // Nothing to read
        fprintf(stderr,"> Error [get_period_pid] - '/tmp/period.pid' is empty\n");
        fclose_perror(file);
        exit(EXIT_FAILURE);
    }

    fclose_perror(file);

    return atoi(buf);
}