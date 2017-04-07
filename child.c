#include "includes.h"
//int findid();
mymsg_t message;
struct sigaction act;
int queueid;
sysClock* clockid;
memCtrl* rscid;

int main(int argc, char **argv){
/*	srand(getpid());
	int msgSize = sizeof(message.mtext);
	int time;
	int queueid = initqueue();
	pcb = pcbMem(MAXP);
	int id = findid();
	
	act.sa_handler = childHandler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);

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
	queueid = initqueue();
	clockid = getClock();
	rscid = getCtrl();

	while(!updateClock(10, clockid)){
		//printf("i'm trying %d\n", getpid());
		//sleep(1);
	}


	printf("child pid: %d\n", getpid());
	return 0;
}

void childHandler(int sig){
	releaseClock(&clockid, ' ');
	releaseCtrl(&rscid, ' ');
	exit(1);
}
