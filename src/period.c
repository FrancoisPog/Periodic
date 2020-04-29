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
int recv_command(struct command *cmd, size_t id){
    
    int pipe = open("/tmp/period.fifo",O_RDONLY);
    if(pipe == -1){
        perror("open");
        return 4;
    }
    printf("> Pipe opened [RD]\n");
    


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

    if(close(pipe) == -1){
        perror("close");
        return -1;
    }
    printf("> Pipe closed\n");
    
    return 0;
}

int add_command(struct command cmd, struct array *list){
    array_add(&commands_list,cmd);

    // Si start = now executer la commande
    // Programmer une alarme si cette commande est la prochaine
    

    return 0;
}

int search_and_execute_commands(struct array *list){
    // Parcourir la liste et stocker les indices des commandes à executer, puis les executer
    // Reprogrammer une alarme pour la prochaine commande 
}

int execute_command(struct command cmd){
    // Executer une commande 
}

/**
 * Send the array of command to periodic
 */ 
int send_command_array(struct array commands_list){

    int pipe = open("/tmp/period.fifo",O_WRONLY);
    if(pipe == -1){
        perror("open");
        return -1;
    }
    printf("> Pipe opened [WR]\n");

    if(commands_list.size == 0){
        char **tmp = calloc(2,sizeof(char*));
        if(tmp == NULL){
            perror("calloc");
            close(pipe);
            return -1;
        }
        tmp[0] = "No command in the list";
        tmp[1] = NULL;
        send_argv(pipe,tmp);
        free(tmp);
        close(pipe);
        printf("> Pipe closed\n");
        return 0;
    }
    

    char** list = calloc(commands_list.size+1,sizeof(char*));
    if(list == NULL){
        perror("calloc");
        close(pipe);
        return -1;
    }

    for (size_t i = 0; i < commands_list.size ; i++){
        // Get the argv length
        size_t index = 0;
        size_t buffsize = 8 + 5 + 10 + 7 + strlen(commands_list.data[i].name);

        while(commands_list.data[i].args[index] != NULL){
            buffsize += strlen(commands_list.data[i].args[index])+1;
            index++;
        }
                
        char *buff = calloc(buffsize,sizeof(char));
        if(buff == NULL){
            perror("calloc");
            close(pipe);
            return -1;
        }

        //write the command in the buffer
        sprintf(buff,"- [%ld] %li %d %s ",commands_list.data[i].id,commands_list.data[i].next,commands_list.data[i].period,commands_list.data[i].name);

        //write the command's arguments in the buffer
        for(size_t k = 0 ; k < index ; ++k){
            strcat(buff,commands_list.data[i].args[k]);
            if(k != index-1){
                strcat(buff," ");
            }
            
        }


        //add the command in the command list
        list[i] = buff;
    }

    if(send_argv(pipe,list) == -1){
        close(pipe);
        return -1;
    }

    for(size_t i = 0 ; i < commands_list.size ; ++i){
        free(list[i]);
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
    size_t count = 55555;
    struct array commands_list;
    array_create(&commands_list);

    while(1){
        pause();
        if(usr1){
            // When usr1
            struct command cmd;
            // Receive command from periodic
            if(recv_command(&cmd,count++) == -1){
                return 6;
            }
            // Add command in array
            add_command(cmd,&commands_list);
            usr1 = 0;
        }
        if (usr2){
            if(send_command_array(commands_list) == -1){
                return 7;
            }
            usr2=0; 
        }
    }

    
    array_destroy(&commands_list);
    unlink("/tmp/period.fifo");
    unlink("/tmp/period.pid");
     
    return 0;
}
