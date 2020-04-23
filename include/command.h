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

void array_create(struct array *self);

void array_destroy(struct array *self);

void array_add(struct array *self, struct command command);

void array_remove(struct array *self);

void array_print(struct array *self);


#endif