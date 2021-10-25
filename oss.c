#include <errno.h>
#include <stdio.h>
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
#include "sharedHandler.h"
#include "signalHandler.h"

void createChild();

// GLOBALS
enum state{idle, want_in, in_cs};
int opt, timer, nValue, tValue;                     //This is for managing our getopts
int currentConcurrentProcesses = 1;                 //Initialized as 1 since the main program is also a process.
int totalProcessesCreated = 0;                      //number of created process
int waitStatus;                                     //This is for managing our processes
sharedMem *shm;                                     //shared memory object
int semID;                                          //SEMAPHORE ID
char *execArgs[1];                                  //exec arguments

int main(void) {
    printf("inside parent main\n");
    initShm();

    setupinterrupt();
    setupitimer( MAX_SECONDS );
    shm = getSharedMemory();                    //getting shared memory
    while(1) {
        while (currentConcurrentProcesses < 20) {
            if(totalProcessesCreated >= 20)
                break;                          //getting out of the while loop
            createChild();
        }
        break;
    }

    removeShm();
    exit(EXIT_SUCCESS);
}

void createChild(){
    pid_t pid;

    if (currentConcurrentProcesses <= MAX_PROC) {
        if ((pid = fork()) == -1) {
            perror("Failed to create grandchild process\n");
            if (removeShm(shmID, shm) == -1) {
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
            printf("current concurrent process %d: myPID: %ld\n", currentConcurrentProcesses, (long) getpid());
            sleep(1);
            execl("./user", execArgs[0], 1, NULL);
        } else {
            /* the parent of grandchild process */
            if (wait(&waitStatus) == -1) {
                perror("Failed to wait for grandchild\n");
            } else {
                if (WIFEXITED(waitStatus)) {
                    currentConcurrentProcesses--;
                    printf("current concurrent process %d\n", currentConcurrentProcesses);
                    printf("Child process successfully exited with status: %d\n", waitStatus);
                    printf("total processes created: %d\n", totalProcessesCreated);
                    //********************************** EXIT SECTION **********************************************
                    //******************************** REMAINDER SECTION *******************************************
                }
            }
        }

    }
}