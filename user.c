//user.c

#include <stdio.h>
#include <stdlib.h>
#include "sharedHandler.h"
#include "signalHandler.h"

int main(int argc, char ** argv){

    if( setupUserInterrupt() == -1 ){
        perror( "failed to set up a user kill signal.\n");
        return 1;
    }

    printf("Hi! I am Child Process Exec'd\n");
    exit(EXIT_SUCCESS);
}