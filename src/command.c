#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// COMMAND

void command_create(struct command *self, char *name, char **args, int period, time_t start){
    self->name = name;
    self->args = args;
    self->next = start;
    self->period = period;
}


void command_print(struct command *self){
    printf("< %s | %li | %d >\n",self->name,self->next,self->period);
}

// COMMAND

void list_create(struct list *self){
    if(self == NULL){
        return;
    }
    self->first = NULL;
}

void list_destroy(struct list *self){
    if(self == NULL){
        return;
    }

    while(self->first != NULL){
        struct node *next = self->first->next;
        free(self->first);
        self->first = next;
    }
}

void list_add(struct list *self, struct command cmd){
    if(self == NULL){
        return;
    }

    struct node *other = calloc(1,sizeof(struct node));
    other->cmd = cmd;
    other->next = self->first;
    self->first = other;
    

}

void list_remove(struct list *self){

}

struct command* list_getNext(struct list *self){
    struct command *cmd = &(self->first->cmd);
    struct node *curr = self->first->next;

    while(curr != NULL){
        if(curr->cmd.next < cmd->next){
            cmd = &(curr->cmd);
        }
        curr = curr->next;
    }

    cmd->next += cmd->period;

    return cmd;

}

void list_print(struct list *self){
    if(self == NULL){
        return;
    }

    struct node *curr = self->first;
    while(curr != NULL){
        command_print(&(curr->cmd));
        curr = curr->next;
    }
}


