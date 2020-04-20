#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <errno.h>

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
    pid_t pid;
    int n = fread(&pid,sizeof(pid_t),1,file);
    if(n == -1){ // Error while reading
        perror("read");
        return -1;
    }

    if(!n){ // Nothing to read
        fprintf(stderr,"> Error [get_period_pid] - '/tmp/period.pid' is empty\n");
        return -1;
    }
    return pid;
}


int main(){

    printf("%d\n",get_period_pid());

    return 0;           
}