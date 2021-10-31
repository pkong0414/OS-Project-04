//sharedHandler.h

/*  I just want to make this file to simplify our main program.
 *  Aim is to have this handle all the shared Memory as well as Semaphores (if we are using them).
 *
 *  I will also be adding message queue as a way to explore that as an option for handling the scheduling.
 */

#ifndef SHAREDHANDLER_H
#define SHAREDHANDLER_H

#include <stdbool.h>
#include <stdio.h>
#include "config.h"

//macros
#define BUF_LEN 1024
#define SECOND 1000000000                                         //a second is 1000000000 nanosecs
#define MILLI 1000000                                             //a millisecond is 1000000 nanosecs
#define MACRO 1000                                                //a macrosecond is 1000 nanosecs

// shared memory struct
typedef struct sharedTimer{
    long sec;                                                     //seconds
    long ns;                                                      //nanoseconds
} Time;

typedef struct sharedMessage{                                     //Struct contains members for the IPC message Queue
    long type;                                                    //message type
    char msg[BUF_LEN];                                           //message itself.
} Message;

typedef struct pcb{
    Time arriveTime;                                              //the time the process first arrived.
    Time burstTime;                                               //the time used from last burst from the process in the system.
    Time totalCpuTime;                                            //total Cpu Time in the system.
    Time waitTime;                                                //time spent waiting.
    Time totalWait;                                               //total time spent waiting.
    Time sysTime;                                                 //total time process spent in the system.
    pid_t userPID;                                                //Process's user pid
    unsigned int priority;                                        //Process's priority ( either 1 or 2 )
} PCB;

typedef struct sharedMemory{                                      //Process Control Block
    Time sysTime;                                                 //the system time.
    PCB pTable[18];                                         //process table for the PCB
} sharedMem;

// sharedMemory functions
void initShm();                                                   //This function will initialize shared memory.
int removeShm();                                                  //This function will remove shared memory.
sharedMem *getSharedMemory();
PCB *getPTablePCB( int index );
PCB *getPTablePID( pid_t pid );
int getFreePTableIndex();

// semaphore functions
void initSem();                                                   //This is going to initialize our semaphores.
int removeSem();                                                  //This is going to remove our semaphores.
void semWait(struct sembuf semW);                                 //This is for our semaphore wait.
void semSignal(struct sembuf semS);                               //This is for our semaphore signals.
void setsembuf(struct sembuf *s, int num, int op, int flg);
int r_semop(struct sembuf *sops, int nsops);

// message queue functions
int initMsq();
int receiveMsg(Message *message, pid_t pid, int msgid, bool waiting);
int sendMsg(Message *message, char *msg, pid_t pid, int msgid, bool waiting );
int removeMsq();
int getPMsgID();                                                  //Grabbing the parent message queue ID
int getCMsgID();                                                  //Grabbing the child message queue ID

//time handling functions
void addTime(long timeValue, Time* time);
void clearTime( Time *time );
void copyTime(Time *src, Time *dest);

#endif