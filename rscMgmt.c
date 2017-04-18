#include "includes.h"
int shmid1;

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
		control->waitedOn[rscNum]++;
	return false;
}

bool releaseRsc(memCtrl* control, int process, int rscNum){
	if (control->totalR[rscNum] == 0) return false;
	control->requested[process][rscNum]--;
	control->available[rscNum]++;
	return true;
}

void releaseAll(memCtrl* control, int process){
	int i;
/*	printf("releasing: %d, there are %d currently remaining.\n",
			control->requested[process][4],
			control->available[4]);*/
	for (i = 0; i < TOTALRSC; i++){
		control->available[i] += control->requested[process][i];
		control->requested[process][i] = 0;
		if (control->waitList[process] > -1){
			control->waitedOn[control->waitList[process]]--;			
			control->waitList[process] = -1;		
		}
	}
}

int waitRelief(memCtrl* control, int rscNum, int myPid){
	int i;
	for (i = 0; i < MAXP; i++){
		if (control->available[rscNum] == 0) break;
		else if (i == myPid) continue;
		else if (control->waitList[i] == -1) continue;
		else if (control->waitList[i] == rscNum){
			control->waitedOn[rscNum]--;
			control->available[rscNum]--;
			control->requested[i][rscNum]++;
			control->waitList[i] = -1;
			return i;
		}
	}
	return -1;
}

void printMyRsc(memCtrl* control, int process, FILE* fptr){
	int i;
	for (i = 0; i < TOTALRSC; i++){
		fprintf(fptr, "\t%d: %d\n",i, control->requested[process][i]);
	}
}

void printWaitList(memCtrl* control, int* children, FILE* fptr){
	int i, k;
	for (i = 0; i < TOTALRSC; i++){
//		printf("%d remaining of rsc %d of %d\n", control->available[i], i, control->totalR[i]);
	}
	for (i = 0; i < MAXP; i++){
		if (control->waitList[i] != -1 && children[i] != 0){
			fprintf(fptr, "%d is waiting on %d\n", children[i], control->waitList[i]);
		}
	}
/*	for (i = 0; i < MAXP; i++){
		if (children[i] != 0){
			printf("%d attained:\n", children[i]);
			for (k = 0; k < TOTALRSC; k++){
				printf("RSC %d: %d\n", k, control->requested[i][k]);
			}
		}
	}
*/
}

void initRsc(memCtrl* control, FILE* fptr){
	srand(getpid());
	int i;
	for (i = 0; i < TOTALRSC; i++){
		control->waitedOn[i] = 0;
		control->totalR[i] = rand()%10 + 1;
		control->available[i] = control->totalR[i];
		if (rand()%5 == 0) control->totalR[i] = 0;
		fprintf(fptr, "created %d of rsc %d\n", control->totalR[i], i);
	}
	for (i = 0; i < MAXP; i++){
		control->waitList[i] = -1;
	}
}

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
