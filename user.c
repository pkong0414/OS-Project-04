//user.c

#include <stdio.h>
#include <stdlib.h>
#include "sharedHandler.h"
#include "signalHandler.h"


int main(int argc, char ** argv){

    printf("Hi! I am Child Process Exec'd\n");
    exit(EXIT_SUCCESS);
}