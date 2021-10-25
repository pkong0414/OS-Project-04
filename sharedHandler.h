//sharedHandler.h

/*  We just want to make this file to simplify our main program.
 *  Aim is to have this handle all the shared Memory as well as Semaphores (if we are using them).
 *
 */

#ifndef SHAREDHANDLER_H
#define SHAREDHANDLER_H

#include "config.h"

//macros
#define BUF_LEN 1024

// shared memory struct
typedef struct sharedTimer{
    long sec;                                   //seconds
    long ns;                                    //nanoseconds
} Time;

typedef struct sharedMessage{
    long type;
    char *msg[BUF_LEN];
} Message;

typedef struct pcb{
    Time wait;

} PCB;

typedef struct sharedMemory{                    //Process Control Block
    Time sysTime;
    PCB pTable[MAX_PROC];
} sharedMem;


void initShm();       //This function will initialize shared memory.
void initSem();       //This is going to initialize our semaphores.
int removeShm();
int removeSem();                                                    //This is going to remove our semaphores.
void semWait(struct sembuf semW);                                                     //This is for our semaphore wait
void semSignal(struct sembuf semS);                                                   //This is for our semaphore signals
void setsembuf(struct sembuf *s, int num, int op, int flg);
int r_semop(struct sembuf *sops, int nsops);

#endif