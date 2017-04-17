/*
 * system.c
 *
 *  Created on: Apr 6, 2017
 *      Author: gregclimer
 */

#include "system.h"
	mymsg_t message;
	int msgKey;

void errorCheck (int i, char* string){
        if (i < 0){
        fprintf(stderr, "\t\t!!!%d: ", getpid());
        perror(string);// exit(-1);
        }
}

int initqueue(){
        errorCheck(msgKey = ftok("Makefile", 'R'), "ftok");
        errorCheck(msgKey=msgget(msgKey, 0666 | IPC_CREAT), "msgget");
        return msgKey;
}

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
void releaseClock(system_t** ptr, char name){
        if(name == 'd')shmctl(shmid, IPC_RMID, NULL);
        else errorCheck(shmdt(*ptr), "shmdt clock");
        return;
}

void initClock(system_t* clock){
	message.mtype = 4;
	message.mtext[0] = 'k';
	errorCheck(msgsnd(msgKey, &message, MSGSIZE, 0), "init clock msg");
	clock->clock[0] = 0;
	clock->clock[1] = 0;
}

bool updateClock(int increment, system_t* clock){
	if (clock->clock[0] >= 1) return false;

	msgrcv(msgKey, &message, MSGSIZE, 4, 0);

	clock->clock[1] += increment;
	rollOver(clock);
	errorCheck(msgsnd(msgKey, &message, MSGSIZE, 0), "clock message");
	return true;
}

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

bool timeIsUp(system_t* clock){
	if (clock->clock[1] > clock->timer[1] && clock->clock[0] >= clock->timer[0])
		return true;
	else if (clock->clock[0] > clock->timer[0])
		return true;
	else return false;

}

void setTimer(system_t* clock){
	clock->timer[0] = clock->clock[0];
	clock->timer[1] = rand()%(int)pow(10,8) + clock->clock[1];
	rollOver(clock);
}



