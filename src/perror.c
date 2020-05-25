#include "perror.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void *calloc_perror(size_t n, size_t size){
    void *res = calloc(n,size);
    if(res == NULL){
        perror("calloc");
        exit(EXIT_NO);
    }
    return res; 
}

int open_perror(const char *file, int oflag, int modes){
    int res = open(file,oflag,modes);
    if(res == -1){
        perror("open");
        exit(EXIT_NO);
    }
    return res;
}

FILE *fopen_perror(const char *filename, const char *modes){
    FILE *res = fopen(filename,modes);
    if(res == NULL){
        perror("fopen");
        exit(EXIT_NO);
    }
    return res;
}

void fclose_perror(FILE *file){
    if(fclose(file) == -1){
        perror("fclose");
        exit(EXIT_NO);
    }
}

void close_perror(int fd){
    if(close(fd) == -1){
        perror("close");
        exit(EXIT_NO);
    }
}

void dup2_perror(int fd, int fd2){
    if(dup2(fd,fd2) == -1){
        perror("dup2");
        exit(EXIT_NO);
    }
}

void sigaction_perror(int sig, const struct sigaction *action,struct sigaction *oldaction){
    if(sigaction(sig,action,oldaction)==-1){
        perror("sigaction");
        exit(EXIT_NO);
    }
}

void sigemptyset_perror(sigset_t *set){
    if(sigemptyset(set) == -1){
        perror("sigemptyset");
        exit(EXIT_NO);
    }
}

void sigprocmask_perror(int how, const sigset_t *set, sigset_t *oldset){
    if(sigprocmask(how,set,oldset) == -1){
        perror("sigprocmask");
        exit(EXIT_NO);
    }
}

void sigaddset_perror(sigset_t *set, int sig){
    if(sigaddset(set,sig) == -1){
        perror("sigaddset");
        exit(EXIT_NO);
    }
}

ssize_t write_perror(int fd, const void *buf, size_t n){
    int res = write(fd,buf,n);
    if( res == -1){
        perror("write");
        exit(EXIT_NO);
    }
    return res;
}


ssize_t read_perror(int fd,void *buf, size_t nbytes){
    ssize_t res = read(fd,buf,nbytes);
    if(res == -1){
        perror("read");
        exit(EXIT_NO);
    }
    return res;
}

int kill_perror(pid_t pid, int sig){
    int res = kill(pid,sig);
    if(res == -1){
        perror("kill");
        exit(EXIT_NO);
    }
    return res;
}