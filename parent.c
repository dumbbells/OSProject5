#include "includes.h"

void initialFork(int);
void masterHandler(int signum);
bool requestMgmt(mymsg_t* message);
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
	int i;
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
		}
	}

	while (updateClock(1000, sysid)){
		msgrcv(queueid, &message,MSGSIZE, 1, IPC_NOWAIT);
		if (message.mtype == 1)
			if(requestMgmt(&message))
				msgsnd(queueid, &message, MSGSIZE, 0);
	}

	printWaitList(rscid, sysid->children);

	masterHandler(0);
}

bool requestMgmt(mymsg_t* message){
	int rsc, childId, count = 0;
	char temp[6];
	pid_t pid;
	while (message->mtext[count] != ' '){
		temp[count] = message->mtext[count];
		count++;
		temp[count] = '\0';
	}
	count++;
	rsc = atoi(temp);
	while (message->mtext[count] != '\0'){
		temp[count - 3] = message->mtext[count];
		count++;
		temp[count - 3] = '\0';
	}
	pid = atoi(temp);
	for (childId = 0; childId < MAXP; childId++){
		if (sysid->children[childId] == pid){
			break;
		}
	}
	if (requestRsc(rscid, childId, rsc)){
		message->mtype = pid;
		return true;
	}
	return false;
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
	}
	errorCheck(msgctl(queueid, IPC_RMID,0 ), "closing message queue");
	releaseCtrl(&rscid, 'd');
	releaseClock(&sysid, 'd');
//	fclose(fptr);
	printf("final clock %li:%09li \n", sysid->clock[0], sysid->clock[1]);
	if (sig != 0) fprintf(stderr, "program is complete\n");
	else printf("no errors detected\n");
	exit(1);
}
