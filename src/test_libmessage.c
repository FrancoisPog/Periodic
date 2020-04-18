#include "message.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int test_send_recv_string(){
    // Named pipe creation
    if(mkfifo("pipe",0777) == -1){
        perror("mkfifo");
        return 1;
    }

    
    // Fork
    if(fork() == 0){
        // Open the named pipe on "read only"
        int fd = open("pipe",O_WRONLY);
        if(fd == -1){
            perror("open");
            return 2;
        }
         
        // Send a string one the named pipe
        if(send_string(fd,"Hello world !") == -1){
            return 3;
        }

        // Close file descriptor
        if(close(fd) == -1){
            perror("close");
            return 4;
        }
        // Unlink (remove) the pipe
        unlink("pipe");
        exit(0);
    }

    // Open the named pipe on "write only"
    int fd = open("pipe",O_RDONLY);
    if(fd == -1){
        perror("open");
        return 2;
    }
   
    // Read a string on the named pipe
    char *str = recv_string(fd);
    if(str == NULL){
        unlink("pipe");
        return 3;
    }
    // Close file descriptor
    if(close(fd) == -1){
        perror("close");
        return 4;
    }
    // Wait the child's death
    wait(NULL);
    
    // Print the receive string
    printf("%s\n",str);

    // Free string
    free(str);

    // Unlink (remove) the pipe
    unlink("pipe");
    return 0;
}

// MAIN
int main(int argc, char *argv[]){

  return test_send_recv_string();
    
}