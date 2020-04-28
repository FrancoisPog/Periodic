#define _POSIX_C_SOURCE 1
#include "message.h"
#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

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

volatile sig_atomic_t usr1 = 0;
volatile sig_atomic_t usr2 = 0;

/**
 * Handler for SIGUSR1 and SIGUSR2
 */
void hand_sigusr(int sig){
    if(sig == SIGUSR1){
        usr1 = 1;
    }else{
        usr2 = 1;
    }
}

/**
 * Receive a command from periodic
 */ 
int recv_command(int pipe, struct command *cmd, size_t id){
    time_t start;
    int n = read(pipe,&start,sizeof(time_t));
    if(n <= 0){
        perror("read [start]");
        return -1;
    }

    int period;
    n = read(pipe,&period,sizeof(int));
    if(n <= 0){
        perror("read [period]");
        return -1;
    }

    char *name = recv_string(pipe);
    if(name == NULL){
        return -1;
    }
    char **args = recv_argv(pipe);
    if(args == NULL){
        free(name);
        return -1;
    }

    cmd->id = id; 
    cmd->next = start;
    cmd->period = period;
    cmd->name = name;
    cmd->args = args;

    
    return 0;
}

/**
 * Send the array of command to periodic
 */ 
int send_command_array(int pipe, struct array commands_list){

    char** list = calloc(commands_list.size,sizeof(char*));

    for (size_t i = 0; i < commands_list.size ; i++){

        // Get the argv length
        size_t size = 0;
        while(commands_list.data->args[size++] != NULL){}
        size--;

        char* str =""; // Mauvaise allocation
        for(size_t i = 0 ; i < size ; ++i){
            strcat(str,commands_list.data->args[i]);
        }
        
        sprintf(str , "%li %s %s %i %li", commands_list.data->id, commands_list.data->name, str, commands_list.data->period, commands_list.data->next);
        /*
        if(send_string(pipe,str) == -1){
            return -1;
        }*/
        strcpy(list[i], str); // list[i] = str
    /*
        // Write the id in the pipe
        if(write(pipe,&commands_list.data->id,sizeof(size_t)) == -1){
            perror("write");
            return -1;
        }

        // Write the command name in the pipe
        if(send_string(pipe,commands_list.data->name) == -1){
            return -1;
        }

        // Write the command arguments in the pipe
        if(send_argv(pipe,commands_list.data->args) == -1){
            return -1;
        }

        // Write the period in the pipe
        if(write(pipe,&commands_list.data->period,sizeof(int)) == -1){
            perror("write");
            return -1;
        }

        //
        if(write(pipe,&commands_list.data->next,sizeof(time_t)) == -1){
            perror("write");
            return -1;
        }*/

    }
    send_argv(pipe,list);

    return 0;
}

// MAIN 
int main(){
    // Initialization
    if(write_pid() <= 0){
        return 1;
    }

    if(make_pipe() == -1){
        return 2;
    }

    if(make_dir() == -1){
        return 3;
    }

    // Set handler
    struct sigaction sig_usr;
    sigemptyset(&sig_usr.sa_mask);
    sig_usr.sa_handler = hand_sigusr;
    sig_usr.sa_flags = 0;
    
    if(sigaction(SIGUSR1,&sig_usr,NULL) == -1){
        perror("sigaction");
        return 5;
    }
    if(sigaction(SIGUSR2,&sig_usr,NULL) == -1){
        perror("sigaction");
        return 5;
    }

    // Creation commands list
    size_t count = 0;
    struct array commands_list;
    array_create(&commands_list);




    // Open pipe
    int pipe = open("/tmp/period.fifo",O_RDONLY);
    if(pipe == -1){
        perror("open");
        return 4;
    }

    while(1){
        if(usr1){
            // When usr1
            struct command cmd;
            // Receive command from periodic
            recv_command(pipe,&cmd,count++);
            // Add command in array
            array_add(&commands_list,cmd);
            array_print(&commands_list);
            usr1 = 0;
        }
        if (usr2){
            // When usr2
            //send the list of commands to periodic
            send_command_array(pipe,commands_list); // fermer et réouvrir le pipe en lecture pour pouvoir écrire
            usr2=0;
        }
        
        pause();
    }

    
    array_destroy(&commands_list);
    unlink("/tmp/period.fifo");
    unlink("/tmp/period.pid");
     
    return 0;
}
