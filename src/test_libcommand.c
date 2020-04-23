#include "command.h"
#include <stdio.h>



int main(){
    struct list cmd_list;
    list_create(&cmd_list);

    struct command cmd;
    
    command_create(&cmd,"sleep",NULL,20,1000);
    list_add(&cmd_list,cmd);

    command_create(&cmd,"echo",NULL,30,0);
    list_add(&cmd_list,cmd);

    command_create(&cmd,"tar",NULL,300,87687987);
    list_add(&cmd_list,cmd);

    list_print(&cmd_list);

    command_print(list_getNext(&cmd_list));

    list_print(&cmd_list);

    command_print(list_getNext(&cmd_list));

    list_destroy(&cmd_list);

    return 0;
}