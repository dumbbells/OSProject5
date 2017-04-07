/*
 * system.h
 *
 *  Created on: Apr 6, 2017
 *      Author: gregclimer
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <math.h>

typedef struct{
	unsigned int clock[2];
	unsigned int timer[2];
} sysClock;

int initqueue();
sysClock* getClock();
void releaseClock(sysClock** ptr, char name);
void initClock(sysClock* clock);
bool updateClock(int increment, sysClock* clock);
bool rollOver(sysClock*);

typedef struct{
	long mtype;
	char mtext[1];
} mymsg_t;




#endif /* SYSTEM_H_ */
