#ifndef INCLUDE_H_
#define INCLUDE_H_
#define MAXP 18
#define TOTALRSC 20

//int shmRsc;

#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <math.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "system.h"

void errorCheck (int i, char* string);

typedef struct{
	int waitedOn[TOTALRSC];	//array keeping track of which resources are in demand
	int totalR[TOTALRSC];		//total RSC (0 is sharable)
	int requested[MAXP][TOTALRSC];	//how many of each resource each child posesses
	int available[TOTALRSC];	//how many copies of each resource are abailable
	int waitList[MAXP];		//which process is on the wait list for which resource
} memCtrl;

void printMyRsc(memCtrl*, int, FILE*);
int waitRelief(memCtrl* , int, int);
void printWaitList(memCtrl*, int*, FILE*);
bool requestRsc(memCtrl*, int, int);
void initRsc(memCtrl*, FILE*);
memCtrl* getCtrl();
void releaseCtrl(memCtrl**, char);
bool releaseRsc(memCtrl*, int, int);
void releaseAll(memCtrl*, int);
#endif
