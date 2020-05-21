#ifndef COMMAND_H
#define COMMAND_H

#include <time.h>
#include <stddef.h>

struct command{
    size_t id;
    char *name;
    char **args;
    int period;
    time_t next;
};

void command_create(struct command *self,size_t id, char *name, char **args, int period, time_t next);

void command_print(struct command *self);



struct array {
    struct command *data;
    size_t capacity;
    size_t size;
};

int array_create(struct array *self);

void array_destroy(struct array *self);

int array_add(struct array *self, struct command command);

int array_remove(struct array *self, int id);

void array_print(struct array *self);


#endif