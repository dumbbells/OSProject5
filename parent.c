#include "includes.h"

void initialFork(int);
void masterHandler(int signum);
bool requestMgmt(mymsg_t*);
void cleanUp(int);

int x = 4;

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
	}

	printWaitList(rscid, sysid->children);
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
	if (message->mtype == 3){
		printf("%d: requesting rsc %d at %li:%09li\n", pid,
				rsc, sysid->clock[0], sysid->clock[1]);
		if (requestRsc(rscid, childId, rsc)){
			message->mtype = pid;
			//printf("!! %d: request granted!!\n", pid);
			return true;
		}
		printf("\t\t%d: I am blocked!!!!\n", sysid->children[childId]);
	}
	else if(message->mtype == 2){
		if (releaseRsc(rscid, childId, rsc)){
			printf("%d: releasing rsc %d at %li:%09li\n", pid,
					rsc, sysid->clock[0], sysid->clock[1]);
			while ((childId = waitRelief(rscid, rsc)) != -1){
				printf("\t%d: sending %d to %d\n",
						pid, rsc, sysid->children[childId]);
				message->mtype = sysid->children[childId];
				sprintf(message->mtext, "cont");
				msgsnd(queueid, message, MSGSIZE, 0);
			}
			//printf("!! %d: release noted!!\n", pid);
		}
	}
	else if (message->mtype == 1){
		printf("\t !! %d: termination noted!!\n", pid);
		cleanUp(childId);
		releaseAll(rscid, childId);
		for (count = 0; count < TOTALRSC; count++){
			while ((childId = waitRelief(rscid, count)) != -1){
							printf("\t%d: sending %d to %d\n",
									pid, count, sysid->children[childId]);
							message->mtype = sysid->children[childId];
							msgsnd(queueid, message, MSGSIZE, 0);
						}
		}
	}
	message->mtype = 4;
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

void cleanUp(int i){
	if (sysid->children[i] != 0){
		kill(sysid->children[i], SIGINT);
		waitpid(sysid->children[i],0,0);
		//printf("\t\t we caught it!\n");
		sysid->children[i] = 0;
	}
}

void masterHandler(int sig){
	int i;
	
	for (i = 0; i < MAXP; i++){
		cleanUp(i);
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
