#include "command.h"
#include <stdio.h>



int main(){
    struct array cmd_array;
    array_create(&cmd_array);

    struct command cmd;

    char *args[3];
    args[0] = "echo";
    args[1] = "Bonjour";
    args[2] = NULL;
    
    command_create(&cmd,1,"sleep",args,20,1000);
    array_add(&cmd_array,cmd);

    command_create(&cmd,2,"echo",args,30,0);
    array_add(&cmd_array,cmd);

    command_create(&cmd,3,"tar",args,300,87687987);
    array_add(&cmd_array,cmd);

    array_print(&cmd_array);

   

    array_destroy(&cmd_array);

    return 0;
}