#include "includes.h"

void childHandler(int sig);
void initChild();
bool reqRsc();
void setReqTimer(int);

struct sigaction act;
int queueid;
system_t childData;
system_t* sysid;
memCtrl* rscid;

int main(int argc, char **argv){
	int quantum = 1000000;
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

	int rsc = rand()%20;


	while(updateClock(quantum, sysid)){
		if (reqRsc()){
			message.mtype = 1;
			printf("%d: requesting rsc %d at %li:%09li\n ", getpid(),
					rsc, sysid->clock[0], sysid->clock[1]);
			sprintf(message.mtext, "%02d %d", rsc, getpid());
			msgsnd(queueid, &message, MSGSIZE, 0);
			msgrcv(queueid, &message, MSGSIZE, getpid(), 0);
			printf("%d: %s\n",getpid(), message.mtext);
		}
	}

	printf("child pid: %d\n", getpid());
	exit(1);
}

bool reqRsc(){
	if (sysid->clock[0] > childData.clock[0] || (
			sysid->clock[0] == childData.clock[0] &&
			sysid->clock[1] >= childData.clock[1])){
		printf("ask for a resource\n");
		return true;
	}
	return false;
}

void initChild(int quantum){
	setReqTimer(quantum);
}

void setReqTimer(int quantum){
	childData.clock[1] = sysid->clock[1] + (rand()%quantum)/10;
	rollOver(&childData);
	printf("wait until %li:%li\n", childData.clock[0], childData.clock[1]);
}

void childHandler(int sig){
	fprintf(stderr, "goodbye from child %d\n", getpid());
	releaseClock(&sysid, ' ');
	releaseCtrl(&rscid, ' ');
	exit(1);
}
