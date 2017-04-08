/*
 * system.h
 *
 *  Created on: Apr 6, 2017
 *      Author: gregclimer
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_
#define MAXP 18


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <math.h>

typedef struct{
	unsigned long clock[2];
	unsigned long timer[2];
	pid_t children[MAXP + 1];
} system_t;

typedef struct{
	long mtype;
	char mtext[2];
} mymsg_t;

bool timeIsUp(system_t* clock);
int initqueue();
system_t* getSystem();
void releaseClock(system_t** ptr, char name);
void initClock(system_t* clock);
bool updateClock(int increment, system_t* clock);
bool rollOver(system_t*);
//void setTimer(int*);
//void spawn(int[]);

#endif /* SYSTEM_H_ */
