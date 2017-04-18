#include "includes.h"

void initialFork();
void masterHandler(int signum);
bool requestMgmt(mymsg_t*);
void cleanUp(int);
void deadLockCheck();
bool futureAvailable(int, int);
void deadLockResolve(int, int);
int numOfDeadLocksResolved(int);

int x = 2;
int deadLocks = 0;
bool verbose = false;

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
	fptr = fopen("a.out", "w");

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
	initRsc(rscid, fptr);

	for (i = 0; i < x; i++){
		initialFork();
	}

	while (updateClock(100000, sysid)){
		if (msgrcv(queueid, &message, MSGSIZE, -3, IPC_NOWAIT) == MSGSIZE){
			if(requestMgmt(&message)){
				msgsnd(queueid, &message, MSGSIZE, 0);
			}
		}
		if (timeIsUp(sysid)){
			initialFork();
			setTimer(sysid);
		}
	}
	masterHandler(0);
}

int numOfDeadLocksResolved(int process){
	int i, value = 1;
	for (i = 0; i < TOTALRSC; i++){
		if (rscid->waitedOn[i] > 0){
			if (rscid->waitedOn[i] >= rscid->requested[process][i])
				value += rscid->requested[process][i];
			else value += rscid->waitedOn[i];
		}
	}
	printMyRsc(rscid, process, fptr);
	return value;
}

void deadLockResolve(int rsc, int process){
	int i, sysPid = process, numResolved, temp;
	deadLocks++;
	numResolved = numOfDeadLocksResolved(process);
	mymsg_t message;
	pid_t pidToKill;
	for (i = 0; i < MAXP; i++){
		if (i == process) continue;
		if (rscid->waitList[i] != rsc) continue;
		if (numResolved <= (temp = numOfDeadLocksResolved(i))){
			sysPid = i;
			numResolved = temp;
		}
	}
	if (numResolved == 0) numResolved = 1;
	pidToKill = sysid->children[sysPid];
	printWaitList(rscid, sysid->children, fptr);
	fprintf(fptr, "The process to terminate to resolve deadlock is %d for %d resolutions\n", pidToKill, numResolved);
	cleanUp(sysPid);
	releaseAll(rscid, sysPid);
	int deadPid = sysPid;
	for (i = 0; i < TOTALRSC; i++){
		while ((sysPid = waitRelief(rscid, i, deadPid)) != -1){
						fprintf(fptr,"\t\t%d: forced termination", pidToKill);
						fprintf(fptr,"\tsending %d to %d\n",
								i, sysid->children[sysPid]);
						message.mtype = sysid->children[sysPid];
						sprintf(message.mtext, "%02d %d", i, sysid->children[sysPid]);
						msgsnd(queueid, &message, MSGSIZE, 0);
					}
	}
}

bool futureAvailable(int rsc, int myPid){
	int i;
	for (i = 0; i < MAXP; i++){
		if (i == myPid) continue;
		if (rscid->requested[i][rsc] > 0){
			if (rscid->waitList[i] == -1) return true;
		}
	}
	fprintf(fptr,"\t\t !!!Deadlock detected!!!\n");
	fprintf(fptr,"%d is deadlocked waiting for %d\n", sysid->children[myPid], rscid->waitList[myPid]);
	return false;
}

void deadLockCheck(){
	int i, rscWaitOn;
	for (i = 0; i < MAXP; i++){
		if (sysid->children[i] == 0) continue;
		else if (rscid->waitList[i] > -1){
			if (verbose) fprintf(fptr,"%d is blocked\n", sysid->children[i]);
			if (!futureAvailable(rscid->waitList[i], i)) deadLockResolve(rscid->waitList[i], i);
		}
	}
}

bool requestMgmt(mymsg_t* message){
	int rsc, childId, count = 0, sysPid;
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
		if (verbose) fprintf(fptr,"%d: requesting rsc %d at %li:%09li\n", pid,
				rsc, sysid->clock[0], sysid->clock[1]);
		if (requestRsc(rscid, childId, rsc)){
			message->mtype = pid;
			return true;
		}
		if (verbose) fprintf(fptr,"\t\t%d: I am blocked!!!!\n", sysid->children[childId]);
		deadLockCheck();
	}
	else if(message->mtype == 2){
		if (releaseRsc(rscid, childId, rsc)){
			sysPid = childId;
			if (verbose) fprintf(fptr,"%d: releasing rsc %d at %li:%09li\n", pid,
					rsc, sysid->clock[0], sysid->clock[1]);
			while ((childId = waitRelief(rscid, rsc, sysPid)) != -1){
				if (verbose) fprintf(fptr,"\t%d: sending %d to %d\n",
						pid, rsc, sysid->children[childId]);
				message->mtype = sysid->children[childId];
				sprintf(message->mtext, "cont");
				msgsnd(queueid, message, MSGSIZE, 0);
			}
			//printf("!! %d: release noted!!\n", pid);
		}
	}
	else if (message->mtype == 1){
		if (verbose) fprintf(fptr,"\t !! %d: termination noted!!\n", pid);
		cleanUp(childId);
		releaseAll(rscid, childId);
		sysPid = childId;
		for (count = 0; count < TOTALRSC; count++){
			while ((childId = waitRelief(rscid, count, sysPid)) != -1){
							if (verbose) printf("\t%d: sending %d to %d\n",
									pid, count, sysid->children[childId]);
							message->mtype = sysid->children[childId];
							msgsnd(queueid, message, MSGSIZE, 0);
						}
		}
	}
	message->mtype = 4;
	return false;
}

void initialFork(){
	int i = 0;	
	while (sysid->children[i] != 0){
		if (i-1 == MAXP) return;
		i++;
	}
	pid_t childPid;
	switch (childPid = fork()){
        	case -1: perror("problem with fork()ing"); break;
        	case 0: execl("userProcess", "./userProcess", NULL); break;
        	default: sysid->children[i] = childPid;
		if (verbose) fprintf(fptr,"\tSpawning a new userProcess %d\n", childPid);
	}
}

void cleanUp(int i){
	if (sysid->children[i] != 0){
		kill(sysid->children[i], SIGINT);
		waitpid(sysid->children[i],0,0);
		if (verbose) fprintf(fptr,"\t\t we caught %d\n", sysid->children[i]);
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
	fclose(fptr);
	fprintf(fptr,"deadlocks resolved: %d\n", deadLocks);
	if (sig != 0) fprintf(stderr, "program was termed\n");
	else if (verbose) printf("no errors detected\n");
	exit(1);
}
