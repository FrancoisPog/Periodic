#define _POSIX_C_SOURCE 1
#include "perror.h"
#include <stdio.h>
#include <wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>


/**
 * Lauch a command as a daemon
 * argv : The arguments value
 */ 
int daemonize(char *argv[]){
    pid_t pid = fork_perror();

    if(pid == 0){

        if(setsid() == -1){
            perror("setsid");
            exit(1);
        }

        pid_t pidGrandChild = fork_perror();

        if(pidGrandChild == 0){
            printf("daemon : %d\n",getpid());

            umask(0);

            if(chdir("/") == -1){
                perror("chdir");
                exit(2);
            }

            fclose_perror(stdin);
            fclose_perror(stdout);
            fclose_perror(stderr);
                
            
            execvp(argv[1],argv+1);
            perror("execvp");
            exit(1);
        }
        exit(0);
    }

    if(wait(NULL) == -1){
        perror("wait");
        return -1;
    }
    return 0;
}



//MAIN
int main(int argc, char *argv[]){
    if(argc == 1){
        fprintf(stderr,"Error : a command must be specified\nUsage : ./launch_daemon command_absolute_path [args]...\n");
        return 1 ;
    }

    if(argv[1][0] != '/'){
        fprintf(stderr,"Error : the command path must be absolute\nUsage : ./launch_daemon command_absolute_path [args]...\n");
        return 2;
    }

    if(daemonize(argv) == -1){
        return 3;
    }
    return 0;
}