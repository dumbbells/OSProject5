#include "includes.h"

void childHandler(int sig);		//function to detach from all shared mem
void initChild();							//function to set up initial values of child
bool reqRsc();								//function to send message for resources
void setReqTimer();						//sets timer until next request
void timeUp();							//checks to see if it should term or continue

struct sigaction act;
int queueid, quantum = 800000;
system_t childData;						//used this struct twice to hold timers
system_t* sysid = NULL;
memCtrl* rscid = NULL;
mymsg_t message;

int main(int argc, char **argv){
	int i, rsc, rscHeld[TOTALRSC];

	for (i = 0; i < TOTALRSC; i++){
		rscHeld[i] = 0;
	}
	srand(getpid());

	act.sa_handler = childHandler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	alarm(1);				//fail safe

	//attaches to shared memory
	queueid = initqueue();
	sysid = getSystem();
	rscid = getCtrl();

	//initializes data for the child including when to request or release
	initChild(quantum);
	setReqTimer(quantum);

	//loop runs until the system clock is greater than 1 second
	while(updateClock(quantum, sysid)){
		timeUp();							//checks to see if it's time to term
		rsc = rand()%TOTALRSC;	//picks resource at random to request/release
		if (reqRsc()){				//determines if it's time to request or release
			sprintf(message.mtext, "%02d %d", rsc, getpid());
			if (rand()%3 != 0){			//most likely will request with msg type 3
				message.mtype = 3;
				msgsnd(queueid, &message, MSGSIZE, 0);
				msgrcv(queueid, &message, MSGSIZE, getpid(), 0);
				rscHeld[rsc]++;
			}
			else {					//will sometimes release if it's available type 2
				message.mtype = 2;
				if (rscHeld[rsc] > 0){
					rscHeld[rsc]--;
					msgsnd(queueid, &message, MSGSIZE, 0);
				}
			}
			setReqTimer(quantum);	//resets timer for next round of requests
		}
	}
	childHandler(0);		//detaches from all shared mem
}

bool reqRsc(){
	if (sysid->clock[0] > childData.clock[0] || (
			sysid->clock[0] == childData.clock[0] &&
			sysid->clock[1] >= childData.clock[1])){
		return true;
	}
	return false;
}

//determines if process should terminate on it's own terms. Most likely
//will continue on. message type 1 (highest priority) if it terms
void timeUp(){
	if (sysid->clock[0] > childData.timer[0] || (
			sysid->clock[0] == childData.timer[0] &&
			sysid->clock[1] >= childData.timer[1])){
		if (rand()%4 == 0){
			sprintf(message.mtext, "00 %d", getpid());
			message.mtype = 1;
			msgsnd(queueid, &message, MSGSIZE, 0);
			childHandler(0);
		}
		else{
			initChild();		//otherwise set new term time
		}
	}
}

//uses timer from system.h
void initChild(){
	childData.timer[1] = sysid->clock[1] + (rand()%quantum)*1000;
	rollOver(&childData);		//in system.c, carries 1 if necessary
}

//uses clock from system.h
void setReqTimer(){
	childData.clock[1] = sysid->clock[1] + (rand()%quantum)*10;
	rollOver(&childData);
}

void childHandler(int sig){
	if(sysid != NULL) releaseClock(&sysid, ' ');
	if(rscid != NULL) releaseCtrl(&rscid, ' ');
	exit(1);
}
