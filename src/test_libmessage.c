#include "message.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
    
// MAIN
int main(int argc, char *argv[]){

    // Named pipe creation
    if(mkfifo("pipe",0777) == -1){
        perror("mkfifo");
        return 1;
    }

    
    // Fork
    if(fork() == 0){
        // Open the named pipe on "write only"
        int fd = open("pipe",O_WRONLY);
        if(fd == -1){
            perror("open");
            unlink("pipe");
            return 2;
        }

        char **strings =calloc(4,sizeof(char *));
        strings[0] = "Langage du Web";
        // strings[0] = NULL;
        strings[1] = "Système et Programmation Système";
        strings[2] = "Programmation Objet Avancée ";
        strings[3] = NULL;
         
        // Send the array of string in the named pipe
        if(send_argv(fd,strings) == -1){
            free(strings);
            unlink("pipe");
            return 3;
        }

        // Close file descriptor
        if(close(fd) == -1){
            perror("close");
            unlink("pipe");
            return 4;
        }

        // Unlink (remove) the pipe
        unlink("pipe");
        free(strings);
        exit(0);
    }

    // Open the named pipe on "read only"
    int fd = open("pipe",O_RDONLY);
    if(fd == -1){
        perror("open");
        unlink("pipe");
        return 2;
    }
   
    // Read an array in the named pipe
    char **str = recv_argv(fd);
    if(str == NULL){
        unlink("pipe");
        return 3;
    }
    // Close file descriptor
    if(close(fd) == -1){
        perror("close");
        unlink("pipe");
        return 4;
    }
    // Wait the child's death
    wait(NULL);
    
    size_t i = 0;
    while(str[i] != NULL){
        printf("strings[%zd] : %s\n",i,str[i]);
        free(str[i++]);
    }

    // Free string
    free(str);

    // Unlink (remove) the pipe
    unlink("pipe");
    return 0;
}
  
