#!bin/bash
# this is the Makefile for Project3

CC = gcc
CFLAGS = -Wall -g -lm

OSS_SRC = oss.c
OSS_OBJ	= $(OSS_SRC:.c=.o) $(SHARED_OBJ) $(SIGNAL_OBJ)
OSS = oss

SHARED_OBJ = sharedHandler.o
SIGNAL_OBJ = signalHandler.o

USER_SRC = user.c
USER_OBJ = $(USER_SRC:.c=.o) $(SHARED_OBJ) $(SIGNAL_OBJ)
USER = user

TARGETS = $(OSS) $(USER)

all: $(TARGETS)

$(OSS): $(OSS_OBJ)
	$(CC) $(CFLAGS) $(OSS_OBJ) -o $(OSS)

$(USER): $(USER_OBJ)
	$(CC) $(CFLAGS) $(USER_OBJ) -o $(USER)

%.o: %.c
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	/bin/rm '-f' *.o $(TARGETS) *.log