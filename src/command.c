#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// COMMAND

void command_create(struct command *self,size_t id, char *name, char **args, int period, time_t start){
    if(self == NULL || name == NULL || args == NULL){
        return;
    }
    self->id = id;
    self->name = name;
    self->args = args;
    self->next = start;
    self->period = period;
}

void command_print(struct command *self){
    if(self == NULL){
        return;
    }
    
    printf("< [%zd] %li | %i | %s ",self->id,self->next,self->period,self->name);
    size_t i = 0;
    while(self->args[i] != NULL){
        printf(" %s ",self->args[i++]);
    }
    printf(" >\n");
}

// ARRAY

void array_create(struct array *self){
    if(self == NULL){
        return;
    }
    
    self->capacity = 10;
    self->size = 0;
    self->data = calloc(self->capacity, sizeof(struct command));

}

void array_destroy(struct array *self){
    if(self == NULL){
        return;
    }

    free(self->data);
    self->data = NULL;
    self->capacity = 0;
    self->size = 0;
}

void array_add(struct array *self, struct command cmd){
    if(self == NULL){
        return;
    }

    if(self->size == self->capacity){
		self->capacity *= 2;
		struct command *data = calloc(self->capacity, sizeof(struct command));
		memcpy(data, self->data, self->size * sizeof(struct command));
		free(self->data);
		self->data = data;
	}
	self->data[self->size] = cmd;
	self->size ++;
    

}

void array_remove(struct array *self){
    //bonus
}



void array_print(struct array *self){
    if(self == NULL){
        return;
    }
    
   for(size_t i = 0 ; i < self->size ; ++i){
       command_print(&(self->data[i]));
   }
}


