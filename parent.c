#include "includes.h"

void initialFork();
void masterHandler(int signum);
bool requestMgmt(mymsg_t*);
void cleanUp(int);
void deadLockCheck();
bool futureAvailable(int, int);
void deadLockResolve(int, int);
int numOfDeadLocksResolved(int);

int x = 2;			//starting processes
int deadLocks = 0;	//counter for deadlocks
bool verbose = false;

struct sigaction act;
int queueid;
system_t* sysid;
memCtrl* rscid;

FILE* fptr;

int main(int argc, char **argv){
	fprintf(stderr, "Master pid: %d\n", getpid());
	mymsg_t message;
	int i;
	fptr = fopen("a.out", "w");

	while ((i = getopt (argc, argv, "v")) != -1){
	switch (i){
	case 'v': verbose = true; break;
	default: fprintf(stderr, "Invalid command line argument\n");
		exit(-1);
	}}

	act.sa_handler = masterHandler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);
	sigaction(SIGINT, &act, 0);
	sigaction(SIGTERM, &act, 0);
	alarm(10);			//failsafe

	queueid = initqueue();		//initializes and attaches msgqueue
	sysid = getSystem();		//initializes and attaches clock and child array
	rscid = getCtrl();		//initializes and attaches resource manager

	initClock(sysid);
	initRsc(rscid, fptr);

	//spawns originial processes in system
	for (i = 0; i < x; i++){
		initialFork();
	}

	//loop runs until system clock passes 2 seconds
	while (updateClock(100000, sysid)){
		//receives any message of type 1, 2 or 3. 1 is highest priority
		//which signifies process termination
		if (msgrcv(queueid, &message, MSGSIZE, -3, IPC_NOWAIT) == MSGSIZE){
			//if request is granted, sends message to child with type of child's
			//pid
			if(requestMgmt(&message)){
				msgsnd(queueid, &message, MSGSIZE, 0);
			}
		}
		//checks if it's time to spawn another user process
		if (timeIsUp(sysid)){
			initialFork();
			setTimer(sysid);		//sets new timer for next user process
		}
	}
	fprintf(stderr, "deadlocks resolved: %d\n", deadLocks);
	fprintf(fptr,"deadlocks resolved: %d\n", deadLocks);
	fprintf(stderr, "Program complete\n");
	fprintf(stderr, "cleaning everything up...\n");
	masterHandler(0);
}

//function to determine maximum amount of blocked processes terminating
//a particular function will relieve.
int numOfDeadLocksResolved(int process){
	int i, value = 0;
	for (i = 0; i < TOTALRSC; i++){
		//if the process is on the waitlist, it tallies how many processes are waiting against how many resources there are to give
		if (rscid->waitedOn[i] > 0){
			if (rscid->waitedOn[i] >= rscid->requested[process][i])
				value += rscid->requested[process][i];
			else value += rscid->waitedOn[i];
		}
	}
	return value;
}

//funtion to resolve deadlocks
void deadLockResolve(int rsc, int process){
	int i, sysPid = process, numResolved, temp;
	deadLocks++;
	//starts bar at just terminating the deadlocked process
	numResolved = numOfDeadLocksResolved(process);
	mymsg_t message;
	pid_t pidToKill;
	for (i = 0; i < MAXP; i++){
		if (i == process) continue;	//don't evaluate the calling process
		if (rscid->waitList[i] < 0) continue;	//don't check processes not wait listed
		fprintf(fptr, "Resources held by %d:\n", sysid->children[process]);
		printMyRsc(rscid, process, fptr);
		//if the process doesn't have the resource in question, don't evaluate
		if (rscid->requested[i][rsc] < 1) continue;
		//check if it's more beneficial to terminate this process
		if (numResolved <= (temp = numOfDeadLocksResolved(i))){
			sysPid = i;
			numResolved = temp;
		}
	}
	pidToKill = sysid->children[sysPid];
	printWaitList(rscid, sysid->children, fptr);
	fprintf(fptr, "The process to terminate to resolve deadlock is %d for %d resolutions\n", pidToKill, numResolved);
	//force term this process and distribute it's resources to those waiting
	cleanUp(sysPid);
	releaseAll(rscid, sysPid);
	int deadPid = sysPid;
	for (i = 0; i < TOTALRSC; i++){
		while ((sysPid = waitRelief(rscid, i, deadPid)) != -1){
						fprintf(fptr,"\t\t%d: forced termination", pidToKill);
						fprintf(fptr,"\tsending rsc %d to %d\n",
								i, sysid->children[sysPid]);
						message.mtype = sysid->children[sysPid];
						sprintf(message.mtext, "%02d %d", i, sysid->children[sysPid]);
						msgsnd(queueid, &message, MSGSIZE, 0);
					}
	}
}

//if there is a copy of the resource the process is waiting on on a non
//blocked process, there is no dead lock, if not, evaluate the dead lock
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

//check all blocked processes and see if any are deadlocked
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

//This catches and interprets messages from the user processes
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
	//type 3 is a request, if it's available, a message is sent to the child
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
	//type 2 is a release, checks if there are any processes waiting on the
	//resource that was just released and sends it to them
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
		}
	}
	//type 1 is a termination. Reclaims all of the processese resources and
	//distributes them to anyone who was waiting
	else if (message->mtype == 1){
		cleanUp(childId);
		releaseAll(rscid, childId);
		sysPid = childId;
		for (count = 0; count < TOTALRSC; count++){
			while ((childId = waitRelief(rscid, count, sysPid)) != -1){
							if (verbose) fprintf(fptr, "\t%d: sending %d to %d\n",
									pid, count, sysid->children[childId]);
							message->mtype = sysid->children[childId];
							msgsnd(queueid, message, MSGSIZE, 0);
						}
		}
	}
	//set message to type 4 to avoid any errors in the main loop
	message->mtype = 4;
	return false;
}

//forks a child in the lowes slot available in child array
void initialFork(){
	int i = 0;
	while (sysid->children[i] != 0){
		if (i == MAXP) return;
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

//kills any children and waits for their response to continue
void cleanUp(int i){
	if (sysid->children[i] > 0){
		kill(sysid->children[i], SIGINT);
		waitpid(sysid->children[i],0,0);
		if (verbose) fprintf(fptr,"\t\t%d: terminated\n", sysid->children[i]);
		sysid->children[i] = 0;
	}
}

//releases all shared memory and cleans up remaining children. 
void masterHandler(int sig){
	int i;
	for (i = 0; i < MAXP; i++){
		cleanUp(i);
	}
	msgctl(queueid, IPC_RMID,0 );
	releaseCtrl(&rscid, 'd');
	releaseClock(&sysid, 'd');
	fclose(fptr);
	exit(1);
}
