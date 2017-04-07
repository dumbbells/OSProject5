/*
 * system.c
 *
 *  Created on: Apr 6, 2017
 *      Author: gregclimer
 */

#include "system.h"

void errorCheck (int i, char* string){
        if (i < 0){
        fprintf(stderr, "%d: ", getpid());
        perror(string);// exit(-1);
        }
}

int initqueue(){
        int key;
        errorCheck(key = ftok("RCS", 'R'), "ftok");
        errorCheck(key=msgget(key, 0666 | IPC_CREAT), "msgget");
        return key;
}

int shmid;
sysClock* getClock(){
		sysClock *loc;
        key_t key;
        errorCheck(key = ftok("RCS", 'R'), "key");
        errorCheck(shmid = shmget(key, (sizeof(sysClock)),
        		0606 | IPC_CREAT), "shmget sysClock");
        loc = shmat(shmid, (void*)0,0);
        return loc;
}
void releaseClock(sysClock** ptr, char name){
        if(name == 'd')shmctl(shmid, IPC_RMID, NULL);
        else errorCheck(shmdt(*ptr), "shmdt");
        return;
}

void initClock(sysClock* clock){
	clock->clock[0] = 0;
	clock->clock[1] = 0;
}

bool updateClock(int increment, sysClock* clock){
	clock->clock[1] += increment;
	//rollOver(clock);
	if (clock->clock[0] == 1) return true;
	return true;
}

bool rollOver(sysClock* clock){
	if (clock->clock[1] >=(int)pow(10,9)){
		clock->clock[0]++;
		clock->clock[1]-=(int)pow(10,9);
	}
	return true;
}

/*void spawn(int timer[]){
	int last = x;
	if (systemClock[0] > timer[0] ||
		(systemClock[0] == timer[0] && systemClock[1] > timer[1])){
		while (true){
			if (x == MAXP) x = 0;
			x++;
			if (x == last) break;
			if (!status[x]){
				spawned++;
				initialFork(x);
				break;
			}
		}
		setTimer(timer);
	}
}
*/
/*void setTimer(int *timer){
	timer[0] = systemClock[0];
	timer[1] = rand()%(int)pow(10,8) + systemClock[1];
	if (timer[1] >=(int)pow(10,9)){
		timer[0]++;
		timer[1]-=(int)pow(10,9);
	}
	fprintf(fptr, "OSS: %d:%09d New        process will be spawned at     %d%09d\n",
		systemClock[0], systemClock[1], timer[0] , timer[1]);
	lineCount++;
}
*/


