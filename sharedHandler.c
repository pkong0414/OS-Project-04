//sharedHandler.c

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "sharedHandler.h"

//permissions
#define PERM (IPC_CREAT | S_IRUSR | S_IWUSR)

static key_t myKey;
static int semID;
static int shmID;
static sharedMem *shmaddr = NULL;

void initShm(){
    //********************* SHARED MEMORY PORTION ************************

    if((myKey = ftok(".",1)) == (key_t)-1){
        //if we fail to get our key.
        fprintf(stderr, "Failed to derive key from filename:\n");
        exit(EXIT_FAILURE);
    }
    printf("derived key from, myKey: %d\n", myKey);

    if( (shmID = shmget(myKey, sizeof(sharedMem), PERM)) == -1){
        perror("Failed to create shared memory segment\n");
        exit(EXIT_FAILURE);
    } else {
        // created shared memory segment!
        printf("created shared memory!\n");

        if((shmaddr = (sharedMem *)shmat(shmID, NULL, 0)) == (void *) -1) {
            perror("Failed to attach shared memory segment\n");
            if(shmctl(shmID, IPC_RMID, NULL) == -1) {
                perror("Failed to remove memory segment\n");
            }
            exit(EXIT_FAILURE);
        }
        // attached shared memory
        printf("attached shared memory\n");
    }
    //****************** END SHARED MEMORY PORTION ***********************
}

void initSem(){
    if( (semID = semget(myKey, 1, PERM | IPC_CREAT)) == -1){
        perror("Failed to create semaphore with key\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Semaphore created with key\n");
    }
    //setsembuf(&semW, 0, -1, 0);
    //setsembuf(&semS, 0, 1, 0);
}

sharedMem *getSharedMemory(){
    return shmaddr;
}

int removeShm(){
    int error = 0;
    printf("Detaching shared memory...\n");

    if(shmdt(shmaddr) == -1){
        printf("Failed to detach shared memory address\n");
        error = errno;
    }
    if((shmctl(shmID, IPC_RMID, NULL) == -1) && !error){
        printf("Failed to detach shared memory id\n");
        error = errno;
    }
    if(!error){
        return 0;
    }
    errno = error;

    printf("DONE!\n");
    return -1;
}

int removeSem(){
    return semctl(semID, 0, IPC_RMID);
}


void semWait(struct sembuf semW){
    if (semop(semID, &semW, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

void semSignal(struct sembuf semS){
    if (semop(semID, &semS, 1) == -1) {
        perror("semop");
        exit(1);
    }
}

int r_semop(struct sembuf *sops, int nsops){
    while(semop(semID, sops, nsops) == -1){
        if(errno != EINTR){
            return -1;
        }
    }
    return 0;
}

void setsembuf(struct sembuf *s, int num, int op, int flg){
    s->sem_num = (short)num;
    s->sem_op = (short)op;
    s->sem_flg = (short)flg;
    return;
}