#include "includes.h"

void childHandler(int sig);
void initChild();
bool reqRsc();
void setReqTimer();
void timeUp();

struct sigaction act;
int queueid, quantum = 1000000;
system_t childData;
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
	alarm(12);

	queueid = initqueue();
	sysid = getSystem();
	rscid = getCtrl();

	initChild(quantum);
	setReqTimer(quantum);

	//rsc = 4;
	while(updateClock(quantum, sysid)){
		timeUp();
		rsc = rand()%TOTALRSC;
		if (reqRsc()){
			sprintf(message.mtext, "%02d %d", rsc, getpid());
			if (rand()%3 != 0){
				message.mtype = 3;
				msgsnd(queueid, &message, MSGSIZE, 0);
				msgrcv(queueid, &message, MSGSIZE, getpid(), 0);
				rscHeld[rsc]++;
			}
			else {
				message.mtype = 2;
				if (rscHeld[rsc] > 0){
					rscHeld[rsc]--;
					msgsnd(queueid, &message, MSGSIZE, 0);
				}
			}
			setReqTimer(quantum);
			//printf("%d: %s\n",getpid(), message.mtext);
		}
		//else printf("skip\n");
	}

	exit(1);
}

bool reqRsc(){
	if (sysid->clock[0] > childData.clock[0] || (
			sysid->clock[0] == childData.clock[0] &&
			sysid->clock[1] >= childData.clock[1])){
		return true;
	}
	return false;
}

void timeUp(){
	if (sysid->clock[0] > childData.timer[0] || (
			sysid->clock[0] == childData.timer[0] &&
			sysid->clock[1] >= childData.timer[1])){
		//if (true){
		if (rand()%4 == 0){
			sprintf(message.mtext, "00 %d", getpid());
			message.mtype = 1;
			msgsnd(queueid, &message, MSGSIZE, 0);
			childHandler(0);
		}
		else{
			initChild();
			//printf("back in the race! \n");
		}
	}
}

void initChild(){
	childData.timer[1] = sysid->clock[1] + (rand()%quantum)*1000;
	rollOver(&childData);
	//printf("My expiration time is: %li:%09li\n", childData.timer[0], childData.timer[1]);
}

void setReqTimer(){
	childData.clock[1] = sysid->clock[1] + (rand()%quantum)*10;
	rollOver(&childData);
	//printf("wait until %li:%09li\n", childData.clock[0], childData.clock[1]);
}

void childHandler(int sig){
	if(sysid != NULL) releaseClock(&sysid, ' ');
	if(rscid != NULL) releaseCtrl(&rscid, ' ');
	exit(1);
}
