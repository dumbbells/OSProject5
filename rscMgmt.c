#include "includes.h"
int shmid1;

memCtrl* getCtrl();
void releaseCtrl(memCtrl** ptr, char name);

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
/*
int totalR[20];
int requested[18][20];
int available[20];
*/
void initRsc(memCtrl* control){
	srand(getpid());
	int i;
	for (i = 0; i < TOTALRSC; i++){
		control->totalR[i] = rand()%10 + 1;
		control->available[i] = control->totalR[i];

	printf("created %d of rsc %d\n", control->totalR[i], i);
	}
}

bool requestRsc(memCtrl* control, int process, int rscNum){
	if (control->available[rscNum]){
		control->available[rscNum]--;
		control->requested[process][rscNum]++;
		return true;
	}
	return false;
}
