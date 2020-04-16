#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    char *usage = "Usage : ./when time";

    if(argc != 2){
        fprintf(stderr,"Error : One and only one time must be specified\n%s\n",usage);
        return 1;
    }

    time_t time = atol(argv[1]);
    printf("%s",ctime(&time));
    return 0;
}