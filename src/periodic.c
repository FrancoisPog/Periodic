#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURE 1
#include "message.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>

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
    char buf[8] = "";
    int n = fread(buf,sizeof(char),8,file);
    if(n == -1){ // Error while reading
        perror("read");
        if(fclose(file) == -1){
            perror("fclose");
        }
        return -1;
    }

    if(!n){ // Nothing to read
        fprintf(stderr,"> Error [get_period_pid] - '/tmp/period.pid' is empty\n");
        if(fclose(file) == -1){
            perror("fclose");
        }
        return -1;
    }

    if(fclose(file) == -1){
        perror("fclose");
    }

    return atoi(buf);
}



/**
 * Check the arguments and determine the start, the period, the command and its arguments.  
 * Return :
 *      > 0 if arguments are valid
 *      > 1 if arguments aren't valid
 */ 
int check_args(int argc, char *argv[], time_t *start, int *period, char **cmd, char ***args){
    if(cmd == NULL || args == NULL || start == NULL || period == NULL || argv == NULL){
        fprintf(stderr,"> Error [check_args] - At least one of parameters is NULL\n");
        return -1;
    }
    char *usage = "Usage : ./periodic [ start period cdm [arg]... ]" ;

    if(argc == 1){
        return 0;
    }

    if(argc < 4){
        fprintf(stderr,"Error : The number of arguments must be 1 or greater than 3\n%s\n",usage);
        return -1;
    }

    // Get the start value
    char **endptr = calloc(1,sizeof(char *));


    if(strcmp(argv[1],"now") == 0){
        *start = time(NULL);
    }else{
        int fromNow = 0;
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

/**
 * Send a command to period
 */ 
int send_command(char *cmd, char **args, time_t start, int period,int pipe){

    short code = 0;
    if(write(pipe,&code,sizeof(short)) == -1){
        perror("write");
        return -1;
    }

    // Write the start timestamp in the pipe
    if(write(pipe,&start,sizeof(time_t)) == -1){
        perror("write");
        return -1;
    }

    // Write the period in the pipe
    if(write(pipe,&period,sizeof(int)) == -1){
        perror("write");
        return -1;
    }

    // Write the command name in the pipe
    if(send_string(pipe,cmd) == -1){
        return -1;
    }

    // Write the command arguments in the pipe
    if(send_argv(pipe,args) == -1){
        return -1;
    }

    
    return 0;
}

int add_command(int argc, char *argv[],int pipe){
    // check and get arguments values
    time_t start = -1;
    int period = -1;
    char *cmd = NULL;
    char **args = NULL;
    if(check_args(argc, argv, &start, &period,&cmd,&args) == -1){
        return -1;
    }
    // Send the command's information
    if(send_command(cmd,args,start,period,pipe) == -1){
        return -1;
    }

    return 0;
}

int remove_command(size_t command_id,int pipe){
    short code = 1;
    if(write(pipe,&code,sizeof(short)) == -1){
        perror("write");
        return -1;
    }

    printf("id : %zd\n",command_id);

    if(write(pipe,&command_id,sizeof(size_t)) == -1){
        perror("write");
        return -1;
    }

    return 0;
}

/**
 * Receive a command from periodic
 */ 
int recv_command_array(pid_t pid){
    if(kill(pid,SIGUSR2) == -1){
        perror("kill");
        return -1;
    }

    int pipe = open("/tmp/period.fifo",O_RDONLY);
    if(pipe == -1){
        perror("open");
        return -1;
    }
    printf("> Pipe opened [RD]\n");
    

    char** list = recv_argv(pipe);
    if(list == NULL){
        return -1;
    }

    size_t i = 0;
    while(list[i] != NULL){
        printf("%s\n",list[i]);
        free(list[i++]);
    }
    
    free(list);
    if(close(pipe) == -1){
        perror("close");
        return -1;
    }
    printf("> Pipe closed\n");

    return 0;
}


// MAIN
int main(int argc, char *argv[]){

    // printf("pid : %d\n",get_period_pid());
    pid_t pid_period = get_period_pid();
    if(pid_period == 0){
        fprintf(stderr,"Error : Period isn't running\n");
        return 1;
    }

    //if periodic used alone -> get command list
    if(argc == 1){
        // Send the USR2 signal to period
        if(recv_command_array(pid_period) == -1){
            return 3;
        }
    }else{
        // Send the USR1 signal to period
        kill(pid_period,SIGUSR1);
       
        // Pipe opening
        int pipe = open("/tmp/period.fifo",O_WRONLY);
        if(pipe == -1){
            perror("open");
            return 2;
        }
        printf("> Pipe opened [WR] : %d\n",pipe);

        if(!strcmp(argv[1],"remove")){
            printf("remove\n");
            remove_command(atol(argv[2]),pipe);
        }else{
            printf("add\n");
            add_command(argc,argv,pipe);
        }

        if(close(pipe) == -1){
            perror("close");
        }
        printf("> Pipe closed\n");
        
    }
    

    return 0;           
}