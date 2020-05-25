#define _POSIX_C_SOURCE 1
#define _XOPEN_SOURCE 500
#define _DEFAULT_SOURCE 1
#include "message.h"
#include "command.h"
#include "file.h"
#include "perror.h"
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


volatile sig_atomic_t usr1 = 0;
volatile sig_atomic_t usr2 = 0;
volatile sig_atomic_t alrm = 0; 
volatile sig_atomic_t chld = 0;
volatile sig_atomic_t stop = 0;


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
    }else if(sig == SIGCHLD){
        chld = 1;
    }else if(sig == SIGINT || sig == SIGTERM || sig == SIGQUIT){
        stop = 1;
    }

    
}


/**
 * Receive a command from periodic
 */ 
int recv_command(struct command *cmd, size_t id, int pipe){
    
    time_t start;
    int n = read(pipe,&start,sizeof(time_t));
    if(n <= 0){
        perror("read");
        return -1;
    }

    int period;
    n = read(pipe,&period,sizeof(int));
    if(n <= 0){
        perror("read");
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
 * Execute a command
 */ 
int execute_command(struct command cmd, struct array *list){
   
    pid_t pid = fork();
    
    if(pid == 0){
        // Here we can't use libperror because we want to use _exit() instead of exit()
        if(setpgid(0,getpgid(getppid())) == -1){
            perror("setpgid");
            _exit(EXIT_NO);
        }

        sigset_t empty;
        if(sigemptyset(&empty) == -1){
            perror("sigemptyset");
            _exit(EXIT_NO);
        }
        
        if(sigprocmask(SIG_SETMASK,&empty,NULL)==-1){
            perror("sigprocmask");
            _exit(EXIT_NO);
        }

        struct sigaction dflt;
        if(sigemptyset(&dflt.sa_mask) == -1){
            perror("sigemptyset");
            _exit(EXIT_NO);
        }
        dflt.sa_handler = SIG_DFL;
        dflt.sa_flags = 0;

        

        if(sigaction(SIGTERM,&dflt,NULL) == -1){
            perror("sigaction");
            _exit(EXIT_NO);
        }

        if(sigaction(SIGINT,&dflt,NULL) == -1){
            perror("sigaction");
            _exit(EXIT_NO);
        }

        if(sigaction(SIGQUIT,&dflt,NULL) == -1){
            perror("sigaction");
            _exit(EXIT_NO);
        }

        command_redirection('o',cmd.id);
        command_redirection('e',cmd.id);
        command_redirection('i',cmd.id);
        


        execvp(cmd.name,cmd.args);
        perror("execvp");
        array_destroy(list);
        _exit(1);
    }
    return 0;
}

/**
 * Execute the next command(s) to execute and set an alarm after
 */ 
int search_and_execute_commands(struct array *list){
    int timer = 0;
    size_t next_cmd_index;
    while(timer <= 0){
        if(list->size == 0){
            alarm(0);
            return 0;
        }
        next_cmd_index = -1;
        for(size_t i = 0 ; i < list->size ; ++i ){
            if(list->data[i].next <= time(NULL) && list->data[i].next != 0){
                if(execute_command(list->data[i], list) == -1){
                    return -1;
                }
                list->data[i].next = time(NULL) + list->data[i].period;
                if(list->data[i].period == 0){
                    array_remove(list,list->data[i].id);
                    --i;
                    continue;
                }
            }

            if(next_cmd_index == -1 ||  (list->data[i].next < list->data[next_cmd_index].next && list->data[i].next != 0)){
                next_cmd_index = i;
            }
            
            
        }
        if(next_cmd_index == -1){
            continue;
        }
        timer =  list->data[next_cmd_index].next - time(NULL);
    }
    
    alarm((unsigned)timer);

    return 0;
}



/**
 * Add a command in the list and set alarm
 */ 
int add_command(struct command cmd, struct array *list){
    
    if(array_add(list,cmd) == -1){
        return -1;
    }
    
    return search_and_execute_commands(list);
}


int usr1_process(struct array *commands_list){
    short code = -1;
    
    
    int pipe = open_perror("/tmp/period.fifo",O_RDONLY, S_IRUSR | S_IWUSR);
    
    read_perror(pipe,&code,sizeof(short));

    if(!code){
        size_t static count = 0;
        struct command cmd;

        // Receive command from periodic
        if(recv_command(&cmd,count++,pipe) == -1){
            return -1;
        }

        close_perror(pipe);

        // Add command in array
        if(add_command(cmd,commands_list) == -1){
            return -1;
        }

    }else{
        size_t id;
        read_perror(pipe,&id,sizeof(size_t));

        close_perror(pipe);

        if(array_remove(commands_list,id) == -1){
            return -1;
        }

    }

    return 0;
}



/**
 * Send the array of command to periodic
 */ 
int send_command_array(struct array commands_list){

    int pipe = open_perror("/tmp/period.fifo",O_WRONLY, S_IRUSR | S_IWUSR);
    

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
        close_perror(pipe);
        return 0;
    }
    

    char** list = calloc(commands_list.size+1,sizeof(char*));
    if(list == NULL){
        perror("calloc");
        close_perror(pipe);
        return -1;
    }

    for (size_t i = 0; i < commands_list.size ; i++){

        // Get the argv length
        size_t index = 0;
        size_t buffsize = 12 + 19 + 10 + 8 + strlen(commands_list.data[i].name);

        while(commands_list.data[i].args[index] != NULL){
            buffsize += strlen(commands_list.data[i].args[index])+1;
            index++;
        }
                
        char *buff = calloc(buffsize,sizeof(char));
        if(buff == NULL){
            perror("calloc");
            close_perror(pipe);
            return -1;
        }

        

        char* time_str = calloc_perror(20,sizeof(char));
        strftime(time_str,20,"%d/%m/%Y %X",localtime( &commands_list.data[i].next ));
        

        //write the command in the buffer
        sprintf(buff,"- [%ld] | %s | %d | %s ",commands_list.data[i].id,time_str,commands_list.data[i].period,commands_list.data[i].name);

        free(time_str);

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
        close_perror(pipe);
        return -1;
    }

    for(size_t i = 0 ; i < commands_list.size ; ++i){
        free(list[i]);
    }
    free(list);

    close_perror(pipe);
    
    return 0;
}

int check_zombie(int block){
    while(1){
        int wstatus;
        pid_t pid;
        if(!block){
            pid = waitpid(-1,&wstatus,WNOHANG);
        }else{
            pid = wait(&wstatus);
        }
        
        if(pid <= 0){
            if(pid == -1 && errno != ECHILD){
                perror("waitpid");
                return -1;
            }
            break;
        }
        
        if (WIFEXITED(wstatus)) {
            fprintf(stderr,"%d : Normal end, statut :  %d\n",pid, WEXITSTATUS(wstatus));
        } else if (WIFSIGNALED(wstatus)) {
            fprintf(stderr,"%d : End by signal n°%d\n",pid, WTERMSIG(wstatus));
        }
    }
    return 0;
}




void exit_properly(int status, struct array *list){  
    unlink("/tmp/period.pid");
    array_destroy(list);
}




// MAIN 
int main(){

    // Redirection of period I/O 
    if(period_redirection() == -1){
        exit(7);
    }

    
    // Write period pid in the file
    pid_t pid_period = write_pid();

    if(pid_period == -1){
        exit(1);
    }

    // Commands list creation
    struct array commands_list;
    if(array_create(&commands_list) == -1){
        exit(14);
    }

    // Set the exit handler
    on_exit(( void (*)( int , void * ) )exit_properly,&commands_list);

   

    // Pipe creation
    if(make_pipe() == -1){
        exit(2);
    }

    // Directory creation
    if(make_dir() == -1){
        exit(4);
    }

    // Set handler
    struct sigaction sig_usr;
    sigemptyset_perror(&sig_usr.sa_mask);
    sig_usr.sa_handler = hand_sigusr;
    sig_usr.sa_flags = 0;
    
    sigaction_perror(SIGUSR1,&sig_usr,NULL); 
    sigaction_perror(SIGUSR2,&sig_usr,NULL) ;
    sigaction_perror(SIGALRM,&sig_usr,NULL) ;
    sigaction_perror(SIGCHLD,&sig_usr,NULL);
    sigaction_perror(SIGINT,&sig_usr,NULL) ;
    sigaction_perror(SIGTERM,&sig_usr,NULL) ;
    sigaction_perror(SIGQUIT,&sig_usr,NULL);
    
   
    // Set the mask
    sigset_t set;
    sigemptyset_perror(&set);

    sigaddset_perror(&set,SIGUSR1) ;
    sigaddset_perror(&set,SIGUSR2) ;
    sigaddset_perror(&set,SIGALRM);
    sigaddset_perror(&set,SIGCHLD);
    sigaddset_perror(&set,SIGINT);
    sigaddset_perror(&set,SIGTERM);
    sigaddset_perror(&set,SIGQUIT);

    sigprocmask_perror(SIG_BLOCK,&set,NULL);

    // Creation of empty set
    sigset_t empty_set;
    sigemptyset_perror(&empty_set);

    while(!stop){
        
        if(sigsuspend(&empty_set) == -1 && errno != EINTR){
            perror("sigsuspend");
            exit(12);
        }

        if(chld){
            if(check_zombie(0) == -1){
                exit(10);
            }            
            chld = 0;
        }

        if(stop){
            break;
        }

        if(usr1){
            if(usr1_process(&commands_list) == -1){
                exit(13);
            }
            usr1 = 0;
        }
        if (usr2){

            if(send_command_array(commands_list) == -1){
                exit(8);
            }
            usr2 = 0; 
        }
        
        if(alrm){
            alrm = 0;
            if(search_and_execute_commands(&commands_list) == -1){
                exit(9);
            }
            
        }

        
       
    }

    // remove alarm
    alarm(0);

    // Send sigterm to all period child
    kill_perror(0,SIGTERM);

    // zombies elimination
    if(check_zombie(1) == -1){
        exit(11);
    }
    
    // END
    exit(0);
}
