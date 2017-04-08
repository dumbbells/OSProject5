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
	int msgSize = sizeof(message.mtext);

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

	message.mtype = 1;
	message.mtext[0] = 'r';
	message.mtext[1] = '\0';
	//printf("%c\n", message.mtext[0]);
	//msgsnd(queueid, &message, msgSize, 0);

	//printf("I sent it!\n");

	while(!timeIsUp(sysid)){
		updateClock(10000001, sysid);
		//printf("%c\n", message.mtext[0]);
		if (!(sysid->clock[0] % 5000000)){
			message.mtext[0] = 'r';
			msgsnd(queueid, &message, msgSize, IPC_NOWAIT);
			msgrcv(queueid, &message, msgSize, 1, 0);
		}
	}



	//printf("child pid: %d\n", getpid());
	exit(1);
	//return 0;
}

void childHandler(int sig){
	fprintf(stderr, "goodbye from child %d\n", getpid());
	releaseClock(&sysid, ' ');
	releaseCtrl(&rscid, ' ');
	exit(1);
}
