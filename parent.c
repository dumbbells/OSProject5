#include "includes.h"

void initialFork();
//void spawn(int[]);
//void iterate();
//void cleanUp(int);
void masterHandler(int signum);
//void dispatch(int);
//void setTimer(int*);


int x = 4;

struct sigaction act;
int queueid;
sysClock* clockid;
memCtrl* rscid;

FILE* fptr;
char* fileName = "a.out";
int lineCount = 0;

pid_t children[MAXP + 1];


int main(int argc, char **argv){
	int i;
	//fptr = fopen(fileName, "w");

    act.sa_handler = masterHandler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, NULL);	
	sigaction(SIGINT, &act, NULL);	

	queueid = initqueue();
	clockid = getClock();
	rscid = getCtrl();
	
	for (i = 1; i <= x; i++){if(children[i] == 1) continue; initialFork(i);}
	
	initClock(clockid);


	while (!updateClock(100, clockid)){
		printf("%d \n", clockid->clock[1]);
		sleep(1);
	}
	printf("%d \n", clockid->clock[1]);

	

	masterHandler(0);
}

void initialFork(int i){
	if (i-1 == MAXP) return;
	pid_t childPid;
	switch (childPid = fork()){
        	case -1: perror("problem with fork()ing"); break;
        	case 0: execl("userProcess", "./userProcess", NULL); break;
        	default: printf("parent pid: %d\n", getpid());
	}
}



//This will be a model for some of the message handling
/*void dispatch(int i){
	char code;
	mymsg_t message;
	msgSize = sizeof(message.mtext);
	message.mtype = i;
	sprintf(message.mtext, "%010d%c\0", quantum * (pcb[i].priority + 1), 'a');
	fprintf(fptr, "OSS: %d:%09d Scheduling process %02d from %d with quantum %010d\n",
		systemClock[0], systemClock[1], i, pcb[i].priority, 
		quantum * (pcb[i].priority + 1));
	lineCount++;
	errorCheck(msgsnd(queueid, &message,msgSize, 0), "msgsnd");
	
	msgrcv(queueid, &message, msgSize, i + 20, 0);
	code = message.mtext[10];
	message.mtext[10] = '\0';
	systemClock[1] += atoi(message.mtext);
	rollOver();
	fprintf(fptr, "OSS: %d:%09d Receiving  process %02d with a run time of  %s\n",
		systemClock[0], systemClock[1], i, message.mtext);
	switch (code){
		case 'd': cleanUp(i); count++; break;
		case 'i': pcb[i].priority = -1;
		case 'a': if (pcb[i].priority < 2) pcb[i].priority++;
		insert(priors[pcb[i].priority], i);
		fprintf(fptr, "OSS: %d:%09d Sorting    process %02d in queue %d\n", 
			systemClock[0], systemClock[1], i, pcb[i].priority);	
	}
	lineCount+=2;

}
*/

/*void iterate(){
	int timer[2], temp;
	setTimer(timer);
	while (systemClock[0] < 2 && lineCount < 10000){
		systemClock[1] += idleTime[1] += temp = rand()%overHead;
		rollOver();
		if (!isEmpty(priors[0])) dispatch(removeData(priors[0]));
		else if (!isEmpty(priors[1])) dispatch(removeData(priors[1]));
		else if (!isEmpty(priors[2])) dispatch(removeData(priors[2]));
		else systemClock[1] += temp;
		spawn(timer);
	}	
}
*/

/*void cleanUp(int i){
	if (status[i]){
		siginfo_t info;
		waitid(P_PID, pcb[i].pid, &info, WEXITED); 
		status[i] = false;
                fprintf(fptr, "OSS: %d:%09d terming    process %02d with total run time %010d\n",
			systemClock[0], systemClock[1], i, maxTime);
		waitTime += ((systemClock[0] * pow(10,9) + systemClock[1])
			 - pcb[i].creationTime[0] *(10,9) - pcb[i].creationTime[1]
			 - pcb[i].totalTime);
	}
	return;
}
*/
void masterHandler(int sig){
	int i;
	
//	for (i = 1; i <= MAXP; i++){
//	kill(pcb[i].pid, SIGINT);
//	}
	errorCheck(msgctl(queueid, IPC_RMID,0 ), "closing message queue");
	releaseCtrl(&rscid, 'd');
	releaseClock(&clockid, 'd');
//	fclose(fptr);
	printf("program is complete\n");
	exit(1);
}
