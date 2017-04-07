#include "includes.h"
int shmid1;

memCtrl* getCtrl();
void releaseCtrl(memCtrl** ptr, char name);

memCtrl* getCtrl(){
		memCtrl *loc;
        key_t key;
        errorCheck(key = ftok("Makefile", 'R'), "key");
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

