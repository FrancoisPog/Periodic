#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Get the PID of the current period process.
 * Return : 
 *      > 0 if period is not running
 *      > The PID if period is running  
 *      > -1 if error
 */
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

/**
 * Check the arguments and determine the start, the period, the command and its arguments.  
 * Return :
 *      > 0 if arguments are valid
 *      > 1 if arguments aren't valid
 */ 
int check_args(int argc, char *argv[], int *start, int *period, char **cmd, char ***args){
    char *usage = "Usage : ./periodic [ start period cdm [arg]... ]" ;

    if(argc == 1){
        return 0;
    }

    if(argc < 4){
        fprintf(stderr,"Error : The number of arguments must be 1 or greater than 3\n%s\n",usage);
        return -1;
    }

    // Get the start value
    int fromNow = 0;
    char **endptr = calloc(1,sizeof(char *));
    if(argv[1][0] == '+'){
        *start = strtol(argv[1]+1,endptr,10);
        fromNow = 1;
    }else{
        *start = strtol(argv[1],endptr,10);
    }

    if(*endptr != argv[1]+strlen(argv[1])){
        free(endptr);
        fprintf(stderr,"Error : Invalid start format\n%s\n",usage);
        return -1;
    }
    
    time_t now = time(NULL);
    if(fromNow){
        *start += now;
    }

    if(*start < now){
        fprintf(stderr,"Error : The start can't be less than the current time\n%s\n",usage);
        free(endptr);
        return -1;
    }

   
    // Get the period value
    *period = strtol(argv[2],endptr,10);
    if(*endptr != argv[2] + strlen(argv[2])){
        free(endptr);
        fprintf(stderr,"Error : Invalid period format\n%s\n",usage);
        return -1;
    }
    free(endptr);
    if(*period < 0){
        fprintf(stderr,"Error : The period can't be negative\n%s\n",usage);
        return -1;
    }

    // Get the command
    *cmd = argv[3];

    // Get the arguments
    *args = argv+3;


    return 0;



}

int main(int argc, char *argv[]){

    int start = -1, period = -1;
    char *cmd = NULL;
    char **args = NULL;
    if(check_args(argc, argv, &start, &period,&cmd,&args) == -1){
        return 1;
    }

    if(argc != 1){
        printf("start = %i\n",start);
        printf("period = %d\n",period);
        printf("command = %s\n",cmd);
        int i =0;
        while(args[i] != NULL){
            printf("args[%d] = %s\n",i,args[i]);
            ++i;
        }
    }
    return 0;           
}