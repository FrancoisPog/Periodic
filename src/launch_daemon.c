#include <stdio.h>
#include <wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>


int daemonize(char *argv[]){
    pid_t pid = fork();
    if(pid == -1){
        perror("fork");
        return -1;
    }

    if(pid == 0){

        if(setsid() == -1){
            perror("setsid");
            exit(1);
        }

        pid_t pidGrandChild = fork();

        if(pidGrandChild == -1){
            perror("fork");
            exit(2);
        }

        if(pidGrandChild == 0){
            printf("daemon : %d\n",getpid());

            umask(0);

            if(chdir("/") == -1){
                perror("chdir");
                exit(2);
            }

            if(fclose(stdin) || fclose(stdout) || fclose(stderr)){
                perror("fclose");
                exit(3);
            }
            
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



int main(int argc, char *argv[]){

    if(daemonize(argv) == -1){
        return 1;
    }
    return 0;
}