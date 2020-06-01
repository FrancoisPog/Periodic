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

/**
 * Create a command
 * self :   The struct command
 * id   :   The command id
 * name :   The command name
 * args :   The command's argument in array
 * period : The command period  
 * next :   The newt execution timestamp
 */ 
void command_create(struct command *self,size_t id, char *name, char **args, int period, time_t next);

/**
 * Destroy a command
 * self : The command to destroy
 */ 
void command_destroy(struct command *self);

/**
 * Print a command
 * self : The command to print
 */
void command_print(struct command *self);



struct array {
    struct command *data;
    size_t capacity;
    size_t size;
};

/**
 * Create an array
 * self : The array to create
 * Return : 
 *      > void on success 
 *      > exit on syscall failure
 */ 
void array_create(struct array *self);

/**
 * Destroy an array
 * self : The array to destroy
 */ 
void array_destroy(struct array *self);

/**
 * Add a command into an array
 * self : The array used
 * command : The command to add
 * Return : 
 *      > void on success 
 *      > exit on syscall failure
 */ 
void array_add(struct array *self, struct command command);

/**
 * Remove a command from an arrar
 * self : The array used
 * command : The command's id to remove
 * Return : 
 *      > void on success 
 *      > exit on syscall failure
 */ 
void array_remove(struct array *self, int id);

/**
 * Print an array
 * self : The array to print
 */ 
void array_print(struct array *self);


#endif