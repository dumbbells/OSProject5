#include "includes.h"

void initialFork(int);
void masterHandler(int signum);
bool requestMgmt(mymsg_t*);
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

	while (updateClock(100000, sysid)){
		if (msgrcv(queueid, &message, MSGSIZE, -3, IPC_NOWAIT) == MSGSIZE){
			if(requestMgmt(&message)){
				msgsnd(queueid, &message, MSGSIZE, 0);
			}
		}
	//	if (msgrcv(queueid, &message, MSGSIZE, 3, IPC_NOWAIT) == MSGSIZE){
			//requestMgmt(&message);
	//		printf("\t!!release noted!!\n");
	//	}
	}

	masterHandler(0);
}

bool requestMgmt(mymsg_t* message){
	int rsc, childId, count = 0;
	char temp[6];
	pid_t pid;
	while (count < 2){
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
	if (message->mtype == 1){
		if (requestRsc(rscid, childId, rsc)){
			message->mtype = pid;
			return true;
		}
		else printWaitList(rscid, sysid->children);
	}
	else if(message->mtype == 2){
		printf("\t!!release noted!!\n");
		releaseRsc(rscid, childId, rsc);
	}
	else if (message->mtype == 3){
		printf("\t !!termination noted!!\n");
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
	if (sig != 0) fprintf(stderr, "program was termed\n");
	else printf("no errors detected\n");
	exit(1);
}
