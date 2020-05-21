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

void command_destroy(struct command *self){
    self->id = 0;
    free(self->name);
    self->name = NULL;
    size_t i = 0;
    while(self->args[i] != NULL){
        free(self->args[i]);
        self->args[i] = NULL;
        i++;
    }
    free(self->args);
    self->args = NULL;
    self->next = 0;
    self->period = 0;
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

int array_create(struct array *self){
    if(self == NULL){
        return -1;
    }
    
    self->capacity = 10;
    self->size = 0;
    self->data = calloc(self->capacity, sizeof(struct command));
    if(self->data == NULL){
        perror("calloc");
        return -1;
    }
    return 0;

}

void array_destroy(struct array *self){
    if(self == NULL){
        return;
    }
    
    for(size_t i = 0; i < self->size ; ++i){
        command_destroy(&(self->data[i]));
    }
    free(self->data);
    self->data = NULL;
    self->capacity = 0;
    self->size = 0;
}

int array_add(struct array *self, struct command cmd){
    if(self == NULL){
        return -1;
    }

    if(self->size == self->capacity){
		self->capacity *= 2;
		struct command *data = calloc(self->capacity, sizeof(struct command));
        if(data == NULL){
            perror("calloc");
            return -1;
        }
		memcpy(data, self->data, self->size * sizeof(struct command));
		free(self->data);
		self->data = data;
	}
	self->data[self->size] = cmd;
	self->size ++;
    return 0;

}

int array_remove(struct array *self, int id){
    if(self == NULL){
        return -1;
    }

    int found = 0;

    for(size_t i = 0 ; i < self->size ; ++i){
        if(self->data[i].id == id){
            found = 1;
            command_destroy(&(self->data[i]));
        }
        if(found){
            if(i != self->size -1){
                self->data[i] = self->data[i+1];
            }
        }
    }

    if(!found){
        return 0;
    }

    self->size--;

    if(self->capacity > 10 && self->size < (self->capacity)/2){
		self->capacity /= 2;
		struct command *data = calloc(self->capacity, sizeof(struct command));
        if(data == NULL){
            perror("calloc");
            return -1;
        }
		memcpy(data, self->data, self->size * sizeof(struct command));
		free(self->data);
		self->data = data;
	}

    return 0;
    
}



void array_print(struct array *self){
    if(self == NULL){
        return;
    }
    
   for(size_t i = 0 ; i < self->size ; ++i){
       command_print(&(self->data[i]));
   }
}


