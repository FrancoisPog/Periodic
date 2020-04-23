#ifndef COMMAND_H
#define COMMAND_H

#include <time.h>
#include <stddef.h>

struct command{
    char *name;
    char **args;
    int period;
    time_t next;
};

void command_create(struct command *self, char *name, char **args, int period, time_t next);

void command_print(struct command *self);

struct node{
    struct command cmd;
    struct node *next;
};

struct list {
    struct node *first;
};

struct command* list_getNext(struct list *self);

void list_create(struct list *self);

void list_destroy(struct list *self);

void list_add(struct list *self, struct command command);

void list_remove(struct list *self);

void list_print(struct list *self);


#endif