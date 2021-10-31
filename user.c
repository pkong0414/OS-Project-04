//user.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sharedHandler.h"
#include "signalHandler.h"

#define TERMCHANCE 70
#define BLOCKCHANCE 50

sharedMem *ushm;
PCB *pcb;
Message message;

void userInit( char** argv );
void processScheduler();
void block();

int main(int argc, char ** argv){

    if( setupUserInterrupt() == -1 ){
        perror( "failed to set up a user kill signal.\n");
        return 1;
    }

    userInit( argv );

    /*  TO DO:
     *
     *      1. We NEED to create a timer for this user process
     *      2. We want to implement a message queue after and have it syncronize with the oss process.
     *
     *      3. We will implement the random number generator to simulate a user process.
     *
     */
    while(1){
        printf("user: sysTime: %ld\n", pcb->sysTime.ns);
        addTime( MILLI, &pcb->sysTime );
        //we want to use message queues
        if( pcb->sysTime.sec == 1 ){
            block();
            printf("it is 1 second, finishing process\n");
            printf("%s: total System time: %ld\n", argv[0], pcb->sysTime.sec);
            break;
        }
    }

    printf("I am child exec'd now my pid is: %ld\n", (long) getpid());
    exit(EXIT_SUCCESS);
}

void userInit( char** argv ){
    initShm();
    initMsq();
    srand(time(NULL) + getpid());
    ushm = getSharedMemory();
    pcb = getPTablePID(getpid());
}

void block(){
    int r = rand() % 6;
    int s = rand() % 1001;
    //receiving a message from blocked processes
    if( sendMsg(&message, "BLOCK", getpid(), getPMsgID(), false ) == -1 ){
        printf("user: did not send a message\n");
    } else {
        //message sent!
        printf("user %ld: BLOCK msg sent\n", (long)getpid());
    }
}