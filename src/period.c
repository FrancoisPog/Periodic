#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


/**
 * Check if period is already running, otherwise create /tmp/period.pid and write the pid inside
 * Return : 
 *      > The pid 
 *      > 0 if period is already running
 *      > -1 if an error has occured
 */ 
int write_pid(){
    // Get the pid
    pid_t pid = getpid();

    // Check is period.pid already exists
    FILE *file = fopen("/tmp/period.pid","r");
    if(file != NULL){
        fprintf(stderr,"> Error [write_pid] - period is already running\n");
        return 0;
    }
    if(errno != ENOENT){
        perror("fopen");
        return -1;
    }

    // Write the pid inside
    file = fopen("/tmp/period.pid","w");
    if(file == NULL){
        perror("fopen");
        return -1;
    }

    fprintf(file,"%i",pid);

    if(fclose(file) == -1){
        perror("fclose");
        return -1;
    }

    return pid;
}

/**
 * Create the pipe only if it doesn't already exists
 * Return : 
 *      > 0 if success
 *      > 1 if already exists
 *      > -1 if errors
 */ 
int make_pipe(){
    if(mkfifo("/tmp/period.fifo",0777) == -1){
        if(errno !=  EEXIST){
            perror("mkfifo");
            return -1;
        }
        fprintf(stderr,"> Warning [make_pipe]\t- period.pipe already exists\n");
        return 1;  
    }
    return 0;
}

/**
 * Create the directory only if it doesn't already exists
 * Return : 
 *      > 0 if success
 *      > 1 if already exists
 *      > -1 if errors
 */ 
int make_dir(){
    if(mkdir("/tmp/period",0777) == -1){
        if(errno != EEXIST){
            perror("mkdir");
            return -1;
        }
        fprintf(stderr,"> Warning [make_dir]\t- /tmp/period already exists\n");
        return 1;
    }
    return 0;
}

int main(){
    
    if(write_pid() <= 0){
        return 1;
    }

    if(make_pipe() == -1){
        return 2;
    }

    if(make_dir() == -1){
        return 3;
    }


    
    unlink("/tmp/period.fifo");
    unlink("/tmp/period.pid");
     
    return 0;
}