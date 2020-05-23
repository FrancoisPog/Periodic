#define _POSIX_C_SOURCE 1
#define _XOPEN_SOURCE 500
#define _DEFAULT_SOURCE 1
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
 *      > -1 if an error has occured (or period is already running)
 */ 
int write_pid(){
    // Get the pid
    pid_t pid = getpid();

    // Check is period.pid already exists
    FILE *file = fopen("/tmp/period.pid","r");
    if(file != NULL){
        fprintf(stderr,"> Error [write_pid] - period is already running\n");
        fclose(file);
        return -1;
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



int period_redirection(){

    int err = open("/tmp/period.err",O_WRONLY | O_CREAT | O_APPEND,S_IRUSR | S_IWUSR);
    if(err == -1){
        perror("open");
        return -1;
    }

    if(dup2(err,STDERR_FILENO) == -1){
        perror("dup2");
        return -1;
    }

    if(close(err) == -1){
        perror("close");
        return -1;
    }

    int out = open("/tmp/period.out",O_WRONLY | O_CREAT | O_APPEND,S_IRUSR | S_IWUSR);
    if(out == -1){
        perror("open");
        return -1;
    }

    if(dup2(out,STDOUT_FILENO) == -1){
        perror("dup2");
        return -1;
    }

    if(close(out) == -1){
        perror("close");
        return -1;
    }

    return 0;
}

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
 * Redirects the standards file descriptor
 */ 
int command_redirection(char type,size_t cmdId){
    if(type != 'i' && type != 'o' && type != 'e'){
        return -1;
    }

    int fd = -1;
    char buf[32];
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

    int out = open(buf,O_WRONLY | O_CREAT | O_APPEND,S_IRUSR | S_IWUSR);
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

        setpgid(0,getpgid(getppid()));

        sigset_t empty;
        sigemptyset(&empty);
        sigprocmask(SIG_SETMASK,&empty,NULL);

        struct sigaction dflt;
        sigemptyset(&dflt.sa_mask);
        dflt.sa_handler = SIG_DFL;
        dflt.sa_flags = 0;

        

        sigaction(SIGTERM,&dflt,NULL);
        sigaction(SIGINT,&dflt,NULL);
        sigaction(SIGQUIT,&dflt,NULL);

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
    
    
    int pipe = open("/tmp/period.fifo",O_RDONLY);
    if(pipe == -1){
        perror("open");
        return -1;
    }

    if(read(pipe,&code,sizeof(short)) == -1){
        perror("read");
        return -1;
    }

    if(!code){
        size_t static count = 0;
        struct command cmd;

        // Receive command from periodic
        if(recv_command(&cmd,count++,pipe) == -1){
            return -1;
        }

        if(close(pipe) == -1){
            perror("close");
            return -1;
        }

        // Add command in array
        if(add_command(cmd,commands_list) == -1){
            return -1;
        }

    }else{
        size_t id;
        if(read(pipe,&id,sizeof(size_t)) == -1){
            return -1;
        }

        if(close(pipe) == -1){
            perror("close");
            return -1;
        }

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

    int pipe = open("/tmp/period.fifo",O_WRONLY);
    if(pipe == -1){
        perror("open");
        return -1;
    }

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

    // Redirection of period I/O 
    if(period_redirection() == -1){
        exit(7);
    }

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
    if(sigemptyset(&sig_usr.sa_mask) == -1){
        perror("sigemptyset");
        exit(19);
    }
    sig_usr.sa_handler = hand_sigusr;
    sig_usr.sa_flags = 0;
    
    if(sigaction(SIGUSR1,&sig_usr,NULL) 
        || sigaction(SIGUSR2,&sig_usr,NULL) 
        || sigaction(SIGALRM,&sig_usr,NULL) 
        || sigaction(SIGCHLD,&sig_usr,NULL)
        || sigaction(SIGINT,&sig_usr,NULL) 
        || sigaction(SIGTERM,&sig_usr,NULL) 
        || sigaction(SIGQUIT,&sig_usr,NULL)){

        perror("sigaction");
        exit(5);
    }
    
   
    // Set the mask
    sigset_t set;
    if(sigemptyset(&set) == -1){
        perror("sigemptyset");
        exit(15);
    }

    if(sigaddset(&set,SIGUSR1) 
        || sigaddset(&set,SIGUSR2) 
        || sigaddset(&set,SIGALRM)
        || sigaddset(&set,SIGCHLD)
        || sigaddset(&set,SIGINT)
        || sigaddset(&set,SIGTERM)
        || sigaddset(&set,SIGQUIT)){
            
        perror("sigaddset");
        exit(16);
    }

    if(sigprocmask(SIG_BLOCK,&set,NULL) == -1){
        perror("sigprocmask");
        exit(17);
    }

    // Creation of empty set
    sigset_t empty_set;
    if(sigemptyset(&empty_set) == -1){
        perror("sigemptyset");
        exit(18);
    }

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
    kill(0,SIGTERM);

    // zombies elimination
    if(check_zombie(1) == -1){
        exit(11);
    }
    
    // END
    exit(0);
}
