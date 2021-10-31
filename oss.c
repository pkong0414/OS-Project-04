#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "queue.h"
#include "sharedHandler.h"
#include "signalHandler.h"

void createChild();
void initOSS();
void initPClock( PCB *pcb );
void exitOSS();
void ossSimulation();
void assignPid2PTable( int index, pid_t pid);
void handleRunningProcess();
void handleBlockedProcess( PCB *pcb);
void handleTerminateProcess( PCB *pcb );
void handleExpiredProcess( PCB *pcb );

void handleBlock();

// GLOBALS
enum state{idle, want_in, in_cs};
int opt, timer, nValue, tValue;                     //This is for managing our getopts
int currentConcurrentProcesses = 1;                 //Initialized as 1 since the main program is also a process.
int totalProcessesCreated = 0;                      //number of created process
int waitStatus;                                     //This is for managing our processes
sharedMem *shm;                                     //shared memory object
int semID;                                          //SEMAPHORE ID
struct sembuf semW;
struct sembuf semS;
Message message;
PCB *activePCB = NULL;
int allowedProcesses = 1;                           //This is the number we will use to define allowed processes for debug use

int main(int argc, char**argv) {
    printf("inside parent main\n");
    //initializing shared memory and semaphore
    initOSS();

    if( setupUserInterrupt() == -1 ){
        perror( "failed to set up a user kill signal.\n");
        return 1;
    }

    //we'll be parsing our arguments here

    //setting up interrupts after parsing arguments
    if (setupinterrupt() == -1) {
        perror("Failed to set up handler for SIGALRM");
        return 1;
    }

    if (setupitimer(MAX_SECONDS) == -1) {
        perror("Failed to set up the ITIMER_PROF interval timer");
        return 1;
    }

    shm = getSharedMemory();                    //getting shared memory

    ossSimulation();

    exitOSS();
}

void createChild(){
    pid_t pid;
    pid_t childPid;
    char buffer[20];
    int pTableID = getFreePTableIndex();
    if (currentConcurrentProcesses <= MAX_PROC) {
        if ((pid = fork()) == -1) {
            perror("Failed to create grandchild process\n");
            if (removeShm() == -1) {
                perror("Failed to destroy shared memory segment");
            }
            exit(EXIT_FAILURE);
        }
        currentConcurrentProcesses++;
        totalProcessesCreated++;

        // made a child process!
        if (pid == 0) {
            /* the child process */
            //debugging output
            childPid = getpid();                                        //getting child pid
            printf("current concurrent process %d: myPID: %ld\n", currentConcurrentProcesses, (long)childPid);
            execl("./user", "user", NULL);

            //setup a message telling the user process to stop

        }

        /* parent process */

        //we are initializing the PCB inside the PTable while we have a chance.
        printf("parent process. Child's PCB %ld\n", (long)pid);
        printf("parent process. totalProcessesCreated: %d\n", totalProcessesCreated);
        PCB* newPCB;
        printf("pTableID: %d\n", pTableID);
        assignPid2PTable(pTableID, pid);
        sprintf(buffer, "%d\n", pTableID );
        printf("buffer: %s\n", buffer);
        sprintf(buffer, "%d\n", pTableID );
        printf("buffer: %s\n", buffer);
        printf("child's pid: %ld\n", (long)pid);
        if(totalProcessesCreated == 1 ){
            activePCB = getPTablePID(pid);
            printf("oss: activePCB's pid: %ld\n", (long)activePCB->userPID);
            initPClock(activePCB);
        } else {
            newPCB = getPTablePID(pid);
            printf("oss: newPCB's pid: %ld\n", (long)newPCB->userPID);
            initPClock(newPCB);
        }

        return;
    }
}

void initOSS(){
    //initializing shared memory and message queues
    initShm();
    initMsq();
    srand(time(NULL));
}

void initPClock( PCB *pcb ){
    clearTime( &pcb->burstTime);
    clearTime( &pcb->totalCpuTime);
    clearTime( &pcb->waitTime);
    clearTime( &pcb->totalWait);
    clearTime( &pcb->sysTime);
    copyTime( &shm->sysTime, &pcb->arriveTime );
}

void exitOSS(){
    //removing shared memory and message queues
    removeShm();
    removeMsq();
    exit(EXIT_SUCCESS);
}

void ossSimulation(){
    while(1) {
        while (currentConcurrentProcesses < 19) {
            if(totalProcessesCreated >= allowedProcesses)
                break;                          //getting out of the while loop
            //we want to increment the time until it is actually time to create processes
            addTime( MILLI, &shm->sysTime );
            printf( "system Time: %ld\n", shm->sysTime.ns );
            if(shm->sysTime.ns % SECOND == 0){
                createChild();
            }
        }
        handleRunningProcess();


        if (wait(&waitStatus) == -1) {
            perror("Failed to wait for child\n");
        } else {
            //we are waiting for our processes to end
            if (WIFEXITED(waitStatus)) {
                currentConcurrentProcesses--;
                printf("current concurrent process %d\n", currentConcurrentProcesses);
                printf("Child process successfully exited with status: %d\n", waitStatus);
                printf("total processes created: %d\n", totalProcessesCreated);
                //*********************************** EXIT SECTION **********************************************
                //******************************** REMAINDER SECTION ********************************************
            }
        }
        if(totalProcessesCreated >= allowedProcesses){
            //we'll want a condition to break out of our loop
            break;
        }
    }
    return;
}

void assignPid2PTable( int index, pid_t pid){
    if( shm->pTable[index].userPID == 0 )
        shm->pTable[index].userPID = pid;
}

void handleRunningProcess(){
    //receiving a message from blocked processes
    if( activePCB != NULL ) {
        PCB *pcb = activePCB;
        printf("inside the scheduler, looking for... %ld\n", (long) activePCB->userPID);
        if (receiveMsg(&message, (long) activePCB->userPID, getPMsgID(), true) == -1) {
            printf("oss: did not receive a message\n");
        } else {
            //message sent!
            printf("message received!\n");
            if (strcmp(message.msg, "BLOCK") == 0) {
                printf("oss: BLOCK msg received from %ld\n", message.type);
                handleBlockedProcess( pcb );
            }
            else if (strcmp(message.msg, "TERMINATE") == 0) {
                printf("oss: TERMINATE msg received from %ld\n", message.type);
                handleTerminateProcess( pcb );
            }
            else if (strcmp(message.msg, "EXPIRED") == 0) {
                printf("oss: EXPIRED msg received from %ld\n", message.type);
                handleExpiredProcess( pcb );
            }
        }
    }
}

void handleBlockedProcess( PCB* pcb ){

}

void handleTerminateProcess( PCB* pcb ){

}

void handleExpiredProcess( PCB* pcb ){

}

void handleBlock(){


}