#ifndef PERROR_H
#define PERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>



void *calloc_perror(size_t n, size_t size);

int open_perror(const char *file, int oflag, int modes);

FILE *fopen_perror(const char *filename, const char *modes);

void close_perror(int fd);

void fclose_perror(FILE *file);

void dup2_perror(int fd, int fd2);

void sigaction_perror(int sig, const struct sigaction *action,struct sigaction *oldaction);

void sigemptyset_perror(sigset_t *set);

void sigprocmask_perror(int how, const sigset_t *set, sigset_t *oldset);

ssize_t write_perror(int fd, const void *buf, size_t n);

void sigaddset_perror(sigset_t *set, int sig);

ssize_t read_perror(int fd,void *buf, size_t nbytes);

int kill_perror(pid_t pid, int sig);

pid_t fork_perror();


#endif