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
	int totalR[TOTALRSC];
	int requested[MAXP][TOTALRSC];
	int available[TOTALRSC];
	int waitList[MAXP];
} memCtrl;

void printWaitList(memCtrl* control, int* children);
bool requestRsc(memCtrl* control, int process, int rscNum);
void initRsc(memCtrl*);
memCtrl* getCtrl();
void releaseCtrl(memCtrl** ptr, char name);
#endif
