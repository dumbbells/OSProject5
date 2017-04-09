#include "includes.h"
//int findid();

void childHandler(int sig);


struct sigaction act;
int queueid;
system_t* sysid;
memCtrl* rscid;

int main(int argc, char **argv){
/*	srand(getpid());
	int msgSize = sizeof(message.mtext);
	int time;
	int queueid = initqueue();
	pcb = pcbMem(MAXP);
	int id = findid();
	


	bool inter, cont = true;	
	
	while (cont){	
		inter = false;
		int temp;
		msgrcv(queueid, &message, msgSize, id, 0);
		message.mtext[10] = '\0';
		message.mtype = id + 20;
		
		time = atoi(message.mtext);
		
		if (!(rand()%3)){
			time = rand() % time;
			inter = true;
		}
		temp = pcb[id].timeUsed;
		pcb[id].timeUsed += time;
		if (pcb[id].timeUsed >= pcb[id].totalTime){
			time = pcb[id].totalTime - temp;
			sprintf(message.mtext, "%010d%c", time, 'd');
			cont = false;
		}
		else if (inter)
			 sprintf(message.mtext, "%010d%c", time, 'i');
		else sprintf(message.mtext, "%010d%c", time, 'a');
	
		msgsnd(queueid, &message, msgSize, 0);
	}
	releasepcb(&pcb, "none");
*/
	mymsg_t message;

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


	while(updateClock(1000000, sysid)){
		if (!(sysid->clock[0] % 100000)){
			message.mtype = 1;
			sprintf(message.mtext, "02 %d", getpid());
			msgsnd(queueid, &message, MSGSIZE, 0);
			msgrcv(queueid, &message, MSGSIZE, getpid(), 0);
			printf("%d: %s\n",getpid(), message.mtext);
		}
	}

	printf("child pid: %d\n", getpid());
	exit(1);
}

void childHandler(int sig){
	fprintf(stderr, "goodbye from child %d\n", getpid());
	releaseClock(&sysid, ' ');
	releaseCtrl(&rscid, ' ');
	exit(1);
}
