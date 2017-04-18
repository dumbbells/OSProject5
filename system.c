/*
 * system.c
 *
 *  Created on: Apr 6, 2017
 *      Author: gregclimer
 */

#include "system.h"
	mymsg_t message;
	int msgKey;

//checks for bad return types
void errorCheck (int i, char* string){
        if (i < 0){
        fprintf(stderr, "\t\t!!!%d: ", getpid());
        perror(string);
        }
}

//creates message queue or returns id for created msg queue
int initqueue(){
        errorCheck(msgKey = ftok("Makefile", 'R'), "ftok");
        errorCheck(msgKey=msgget(msgKey, 0666 | IPC_CREAT), "msgget");
        return msgKey;
}

//system clock structure for tracking time and children
int shmid;
system_t* getSystem(){
		system_t *loc;
        key_t key;
        errorCheck(key = ftok("Makefile", 'R'), "key");
        errorCheck(shmid = shmget(key, (sizeof(system_t)),
        		0606 | IPC_CREAT), "shmget sysClock");
        loc = shmat(shmid, (void*)0,0);
        return loc;
}
//detaches and/or destroys system clock
void releaseClock(system_t** ptr, char name){
        if(name == 'd')shmctl(shmid, IPC_RMID, NULL);
        else shmdt(*ptr);
        return;
}

//initializes clock and begins churn by sending a message of type 4
void initClock(system_t* clock){
	message.mtype = 4;
	message.mtext[0] = 'k';
	errorCheck(msgsnd(msgKey, &message, MSGSIZE, 0), "init clock msg");
	clock->clock[0] = 0;
	clock->clock[1] = 0;
}

//critical section, all processes must receive the one message of
//type 4 before they can increment the clock.
bool updateClock(int increment, system_t* clock){
	if (clock->clock[0] > 1) return false;

	msgrcv(msgKey, &message, MSGSIZE, 4, 0);

	clock->clock[1] += increment;
	rollOver(clock);
	errorCheck(msgsnd(msgKey, &message, MSGSIZE, 0), "clock message");
	return true;
}

//function to carry the 1 on the clock
bool rollOver(system_t* clock){
	if (clock->clock[1] >=(int)pow(10,9)){
		clock->clock[0]++;
		clock->clock[1]-=(int)pow(10,9);
	}
	if (clock->timer[0] >=(int)pow(10,9)){
		clock->timer[0]++;
		clock->timer[1]-=(int)pow(10,9);
	}
	return true;
}

//checks to see if it's time to spawn a new process or make a request
bool timeIsUp(system_t* clock){
	if (clock->clock[1] > clock->timer[1] && clock->clock[0] >= clock->timer[0])
		return true;
	else if (clock->clock[0] > clock->timer[0])
		return true;
	else return false;

}

//function to set timer for a new process
void setTimer(system_t* clock){
	clock->timer[0] = clock->clock[0];
	clock->timer[1] = rand()%(int)pow(10,8) + clock->clock[1];
	rollOver(clock);
}
