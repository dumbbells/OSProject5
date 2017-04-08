#include "includes.h"

void initialFork(int);
void masterHandler(int signum);
void requestMgmt(mymsg_t message);
//void cleanUp(int);

int x = 2;

struct sigaction act;
int queueid;
system_t* sysid;
memCtrl* rscid;

FILE* fptr;
char* fileName = "a.out";
int lineCount = 0;

int main(int argc, char **argv){
	mymsg_t message;
	int i, msgSize = sizeof(message.mtext);
	//fptr = fopen(fileName, "w");

    act.sa_handler = masterHandler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);
	sigaction(SIGINT, &act, 0);
	sigaction(SIGTERM, &act, 0);
	alarm(10);

	queueid = initqueue();
	sysid = getSystem();
	rscid = getCtrl();
	
	initClock(sysid);
	initRsc(rscid);

	for (i = 0; i < x; i++){
		if (sysid->children[i] == 0){
			initialFork(i);
			//printf("%d: %d\n", i, sysid->children[i]);
		}
	}

	while (!timeIsUp(sysid)){
		updateClock(10000000, sysid);
		msgrcv(queueid, &message, msgSize, 1, IPC_NOWAIT);
		fflush(stdout);
		//fprintf(stderr,"%c\n", message.mtext[0]);
		//printf("we got one %c\n", message.mtext[0]);
		if (message.mtext[0] == 'r'){
			printf("%c i got it!\n", message.mtext[0]);
		}
		message.mtext[0] = 'O';
		//requestMgmt(message);
		//message.mtext[0] = '\0';
		//printf("nothing\n");
		msgsnd(queueid, &message, msgSize, 0);
	}
	//printf("%d\n", (int)sizeof(unsigned long));


	sleep(2);
	printf("final clock %li:%09li \n", sysid->clock[0], sysid->clock[1]);
	masterHandler(0);
}

void requestMgmt(mymsg_t message){
	if (message.mtext[0] == 'r'){
		printf("%c\n", message.mtext[0]);
	}
	//printf("found\n");
	//message.mtext[0] = 'k';
	//msgsnd(queueid, &message, msgSize, 0);
}

void initialFork(int i){
	if (i-1 == MAXP) return;
	pid_t childPid;
	switch (childPid = fork()){
        	case -1: perror("problem with fork()ing"); break;
        	case 0: execl("userProcess", "./userProcess", NULL); break;
        	default: sysid->children[i] = childPid;
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
	
	for (i = 0; i < MAXP; i++){
		if (sysid->children[i]){
			kill(sysid->children[i], SIGINT);
			waitpid(sysid->children[i],0,0);
		}
		//else printf("no child %d\n", i);
	}
	errorCheck(msgctl(queueid, IPC_RMID,0 ), "closing message queue");
	releaseCtrl(&rscid, 'd');
	releaseClock(&sysid, 'd');
//	fclose(fptr);
	if (sig != 0) fprintf(stderr, "program is complete\n");
	else printf("no errors detected\n");
	exit(1);
}
