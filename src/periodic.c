#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURE 1
#include "message.h"
#include "file.h"
#include "perror.h"
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
 * Check the arguments and determine the start, the period, the command and its arguments.  
 * argc : The number of arguments
 * argv : The arguments values
 * cmd,args,start,period : All command's data to set
 * Return :
 *      > 0 if arguments are valid
 *      > 1 if arguments aren't valid
 */ 
int check_args(int argc, char *argv[], time_t *start, int *period, char **cmd, char ***args){
    if(cmd == NULL || args == NULL || start == NULL || period == NULL || argv == NULL){
        fprintf(stderr,"> Error [check_args] - At least one of parameters is NULL\n");
        return -1;
    }
    char *usage = "Usage : ./periodic\nUsage : ./periodic start period cmd [arg]...\nUsage : ./periodic remove command_id" ;

    if(argc == 1){
        return 0;
    }

    if(argc < 4){
        fprintf(stderr,"Error : Invalid arguments number\n%s\n",usage);
        return -1;
    }

    // Get the start value
    char **endptr = calloc_perror(1,sizeof(char *));
    
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
 * cmd,args,start,period : All command's data
 * pipe : The pipe used
 * Return : 
 *      > 0 on success
 *      > -1 on errors
 *      > exit on syscall failure
 */ 
int send_command(char *cmd, char **args, time_t start, int period,int pipe){

    short code = 0;
    write_perror(pipe,&code,sizeof(short));
        
    // Write the start timestamp in the pipe
    write_perror(pipe,&start,sizeof(time_t));

    // Write the period in the pipe
    write(pipe,&period,sizeof(int));

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

/**
 * Execute the process to send a command to period
 * argc : The number of arguments
 * argv : The arguments values
 * pid_period : The current period pid
 * Return : 
 *      > 0 on success
 *      > -1 on errors
 *      > exit on syscall failure
 */ 
int add_command(int argc, char *argv[],int pid_period){
    // check and get arguments values
    time_t start = -1;
    int period = -1;
    char *cmd = NULL;
    char **args = NULL;
    if(check_args(argc, argv, &start, &period,&cmd,&args) == -1){
        return -1;
    }

    // Send the USR1 signal to period
    kill_perror(pid_period,SIGUSR1);

    // Pipe opening
    int pipe = open_perror("/tmp/period.fifo",O_WRONLY, S_IRUSR | S_IWUSR);
    
    // Send the command's information
    if(send_command(cmd,args,start,period,pipe) == -1){
        return -1;
    }

    close_perror(pipe);

    return 0;
}

/**
 * Execute the process to send the command's id to remove to period
 * command_id_str : The id of command to remove in String
 * pid_period : The current period pid
 * Return : 
 *      > 0 on success
 *      > -1 on errors
 *      > exit on syscall failure
 */ 
int remove_command(char *command_id_str,int pid_period){
    char *usage = "Usage : ./periodic\nUsage : ./periodic start period cmd [arg]...\nUsage : ./periodic remove command_id" ;
    if(command_id_str == NULL){
        fprintf(stderr,"Error : The command id is missing\n%s\n",usage);
        return -1;
    }
    
    char **endptr = calloc_perror(1,sizeof(char *));
   
    ssize_t command_id = strtoll(command_id_str,endptr,10);

    if(*endptr != command_id_str + strlen(command_id_str)){
        free(endptr);
        fprintf(stderr,"Error : Invalid id format\n%s\n",usage);
        return -1;
    }
    free(endptr);

    if(command_id < 0){
        fprintf(stderr,"Error : The command id can't be negative\n%s\n",usage);
        return -1;
    }

    // Send the USR1 signal to period
    kill_perror(pid_period,SIGUSR1);

    // Pipe opening
    int pipe = open_perror("/tmp/period.fifo",O_WRONLY,S_IRUSR | S_IWUSR);

    // Write the process code
    short code = 1;
    write_perror(pipe,&code,sizeof(short));

    // Write the id
    write_perror(pipe,&command_id,sizeof(size_t));

    close_perror(pipe);

    return 0;
}

/**
 * Request the command array from periodic and print it
 * pid : The current period pid
 * Return :
 *      > 0 on success
 *      > -1 on errors
 *      > exit on syscall failure
 */ 
int recv_command_array(pid_t pid){
    // Send sigusr2 to period
    kill_perror(pid,SIGUSR2);

    // Pipe opening
    int pipe = open_perror("/tmp/period.fifo",O_RDONLY,S_IRUSR | S_IWUSR);
    
    // Receive the array
    char** list = recv_argv(pipe);
    if(list == NULL){
        return -1;
    }

    // Print the actual time
    char* now_str = calloc_perror(20,sizeof(char));
    time_t now = time(NULL);
    strftime(now_str,20,"%d/%m/%Y %X",localtime(&now));
    printf("Actual time : %s\n",now_str);
    free(now_str);

    // Print the array
    size_t i = 0;
    while(list[i] != NULL){
        printf("%s\n",list[i]);
        free(list[i++]);
    }
    
    free(list);
    close_perror(pipe);
    return 0;
}


// MAIN
int main(int argc, char *argv[]){
    
    // Get the period pid
    pid_t pid_period = get_period_pid();
    if(pid_period == 0){
        fprintf(stderr,"Error : Period isn't running\n");
        return 1;
    }

    //If periodic used alone -> get command list
    if(argc == 1){
        // Send the USR2 signal to period
        if(recv_command_array(pid_period) == -1){
            return 2;
        }
    }else{
        // If the first argument is 'remove'
        if(!strcmp(argv[1],"remove")){
            if(remove_command(argv[2],pid_period) == -1){
                return 3;
            }
        }else{
            if(add_command(argc,argv,pid_period) == -1){
                return 4;
            }
        }
    }
    return 0;           
}