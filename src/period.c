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
#include <sys/wait.h>
#include <assert.h>



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
volatile sig_atomic_t alrm = 0;

/**
 * Handler for SIGUSR1 and SIGUSR2
 */
void hand_sigusr(int sig){
    if(sig == SIGUSR1){
        usr1 = 1;
    }else if(sig == SIGUSR2){
        usr2 = 1;
    }else if(sig == SIGALRM){
        alrm = 1;
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

/**
 * Redirects the standards file descriptor
 */ 
int redirection(char type,size_t cmdId){
    if(type != 'i' && type != 'o' && type != 'e'){
        return -1;
    }

    int fd = -1;
    char buf[23];
    switch(type){
        case 'i' : {
            sprintf(buf,"/dev/null");
            fd = STDIN_FILENO;
            break;
        }
        case 'o' : {
            sprintf(buf,"/tmp/period/%zd.out",cmdId);
            fd = STDOUT_FILENO;
            break;
        }
        case 'e' : {
            sprintf(buf,"/tmp/period/%zd.err",cmdId);
            fd = STDERR_FILENO;
            break;
        }
        default : {
            assert(0);
        }
    }
    assert(fd != -1);

    int out = open(buf,O_WRONLY | O_CREAT | O_APPEND,0755);
    if(out == -1){
        perror("open");
        exit(1);
    }

    if(dup2(out, fd) == -1){
        perror("dup2");
        exit(2);
    }

    if(close(out) == -1){
        perror("close");
        exit(3);
    }

    return 0;
}


/**
 * Execute a command
 */ 
int execute_command(struct command cmd, struct array *list){
    
    pid_t pid = fork();
    if(pid == 0){

        
       
        redirection('o',cmd.id);
        redirection('e',cmd.id);
        redirection('i',cmd.id);
        

        execvp(cmd.name,cmd.args);
        perror("execvp");
        array_destroy(list);
        exit(1);
    }

    return 0;
}

/**
 * Set an alarm for the next command to execute
 */ 
int set_alarm(struct array *list){
    int next_cmd_index = 0;
    for(size_t i = 1 ; i < list->size ; ++i){
        if(list->data[i].next < list->data[next_cmd_index].next){
            next_cmd_index = i;
        }
    }
    unsigned int timer =  list->data[next_cmd_index].next - time(NULL);
    // printf("[Prochaine commande : %s - %ds]\n",list->data[next_cmd_index].name,timer);
    alarm(timer);
    
    return 0;
}

/**
 * Add a command in the list and set alarm
 */ 
int add_command(struct command cmd, struct array *list){

    if(time(NULL) == cmd.next){
        if(execute_command(cmd,list) == -1){
            return -1;
        }
        cmd.next += cmd.period;
    }
    array_add(list,cmd);
    set_alarm(list);
    return 0;
}

/**
 * Execute the next command(s) to execute and set an alarm after
 */ 
int search_and_execute_commands(struct array *list){
    int nothing_to_execute = 1;
    for(size_t i = 0 ; i < list->size ; ++i ){
        if(list->data[i].next == time(NULL)){
            if(execute_command(list->data[i], list) == -1){
                return -1;
            }
            list->data[i].next += list->data[i].period;
            nothing_to_execute = 0;
        }
    }
    assert(!nothing_to_execute);

    set_alarm(list);

    return 0;
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
    if(sigaction(SIGALRM,&sig_usr,NULL) == -1){
        perror("sigaction");
        return 5;
    }
    

    // Creation commands list
    size_t count = 0;
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

        if(alrm){
            if(search_and_execute_commands(&commands_list) == -1){
                return -1;
            }
            alrm = 0;
        }
        
    }

    
    
     
    return 0;
}
