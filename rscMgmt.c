#include "includes.h"
int shmid1;

void printWaitList(memCtrl* control, int* children);
bool requestRsc(memCtrl* control, int process, int rscNum);
void initRsc(memCtrl*);
memCtrl* getCtrl();
void releaseCtrl(memCtrl** ptr, char name);
void releaseRsc(memCtrl* control, int process, int rscNum);

memCtrl* getCtrl(){
		memCtrl *loc;
        key_t key;
        errorCheck(key = ftok("includes.h", 'R'), "key");
        errorCheck(shmid1 = shmget(key, (sizeof(memCtrl)),
        		0606 | IPC_CREAT), "shmget memCtrl");
        loc = shmat(shmid1, (void*)0,0);
        return loc;
}

void releaseCtrl(memCtrl** ptr, char name){
        if(name == 'd') shmctl(shmid1, IPC_RMID, NULL);
        else errorCheck(shmdt(*ptr), "shmdt");
        return;
}

void initRsc(memCtrl* control){
	srand(getpid());
	int i;
	for (i = 0; i < TOTALRSC; i++){
		control->totalR[i] = rand()%10 + 1;
		control->available[i] = control->totalR[i];
		//if (rand()%5 == 0) control->totalR[i] = 0;
		if (i < MAXP) control->waitList[i] = -1;
		printf("created %d of rsc %d\n", control->totalR[i], i);
	}
}

bool requestRsc(memCtrl* control, int process, int rscNum){
	if (control->totalR[rscNum] == 0)
		return true;
	else if (control->available[rscNum]){
		control->available[rscNum]--;
		control->requested[process][rscNum]++;
		return true;
	}
	else
		control->waitList[process] = rscNum;
	return false;
}

void releaseRsc(memCtrl* control, int process, int rscNum){
	if (control->totalR[rscNum] == 0) return;
	control->requested[process][rscNum]--;
	control->available[rscNum]++;
}

void printWaitList(memCtrl* control, int* children){
	int i;
	for (i = 0; i < MAXP; i++){
		if (control->waitList[i] != -1){
			printf("%d is waiting on %d\n", children[i], control->waitList[i]);
		}
	}
}
