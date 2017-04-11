#include "includes.h"

void childHandler(int sig);
void initChild();
bool reqRsc();
void setReqTimer();
void timeUp();

struct sigaction act;
int queueid, quantum = 1000000;
system_t childData;
system_t* sysid;
memCtrl* rscid;

int main(int argc, char **argv){
	int i, rscHeld[TOTALRSC];

	for (i = 0; i < TOTALRSC; i++){
		rscHeld[i] = 0;
	}
	mymsg_t message;
	srand(getpid());

	act.sa_handler = childHandler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	alarm(10);

	queueid = initqueue();
	sysid = getSystem();
	rscid = getCtrl();

	initChild(quantum);
	setReqTimer(quantum);

	int rsc = rand()%TOTALRSC;


	while(updateClock(quantum, sysid)){
		if (reqRsc()){
			if (rand()%3 != 0){
				message.mtype = 1;
				printf("%d: requesting rsc %d at %li:%09li\n ", getpid(),
						rsc, sysid->clock[0], sysid->clock[1]);
				sprintf(message.mtext, "%02d %d", rsc, getpid());
				msgsnd(queueid, &message, MSGSIZE, 0);
				msgrcv(queueid, &message, MSGSIZE, getpid(), 0);
				rscHeld[rsc]++;
			}
			else {
				printf("release\n");
				message.mtype = 3;
				for (i = 0; i < TOTALRSC; i++){
					if (rscHeld[i] != 0){
						while (true){
							rsc = rand()%TOTALRSC;
							if(rscHeld[rsc] != 0){
								i = 20;
								break;
							}
						}
					}
				}
				printf("%d: releasing rsc %d at %li:%09li\n ", getpid(),
						rsc, sysid->clock[0], sysid->clock[1]);
				sprintf(message.mtext, "%02d %d", rsc, getpid());
				msgsnd(queueid, &message, MSGSIZE, 0);
			}
			setReqTimer(quantum);
			//printf("%d: %s\n",getpid(), message.mtext);
		}
		//else printf("skip\n");
	}

	printf("child pid: %d\n", getpid());
	exit(1);
}

bool reqRsc(){
	timeUp();
	if (sysid->clock[0] > childData.clock[0] || (
			sysid->clock[0] == childData.clock[0] &&
			sysid->clock[1] >= childData.clock[1])){
		printf("ask for attention\n");
		return true;
	}
	return false;
}

void timeUp(){
	if (sysid->clock[0] > childData.timer[0] || (
			sysid->clock[0] == childData.timer[0] &&
			sysid->clock[1] >= childData.timer[1])){
		if (rand()%4 == 0){
			printf("on my own accord\n");
			childHandler(0);
		}
		else{
			initChild();
			printf("back in the race! \n");
		}
	}
}

void initChild(){
	childData.timer[1] = sysid->clock[1] + (rand()%quantum)*100;
	rollOver(&childData);
	printf("My expiration time is: %li:%09li\n", childData.timer[0], childData.timer[1]);
}

void setReqTimer(){
	childData.clock[1] = sysid->clock[1] + (rand()%quantum)*10;
	rollOver(&childData);
	printf("wait until %li:%09li\n", childData.clock[0], childData.clock[1]);
}

void childHandler(int sig){
	fprintf(stderr, "goodbye from child %d\n", getpid());
	releaseClock(&sysid, ' ');
	releaseCtrl(&rscid, ' ');
	exit(1);
}
