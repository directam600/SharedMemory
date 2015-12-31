/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
     A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - JAMES DU */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>

typedef struct shmem{
	int bits[8000];
	int perfect[20];
	int process[20][4];
} shm;

struct msgbuf{
	long mtype;
	int num;
};

void signal_setup();
void signal_handler();

shm *ptr;
struct msgbuf *msger;
struct msgbuf *msger2;
struct msgbuf *msger3;

int main(int argc, char *argv[]){
	int id, msg, pid, N;	//msg used for pid, msg2 used for perfect numbers
	int mngid = getpid();
	int perfcount = 0;
	int counter = 0;
	bool check = true;
	key_t key = 85108;
	msger = (struct msgbuf*)malloc(sizeof(*msger));
	msger2 = (struct msgbuf*)malloc(sizeof(*msger2));
	msger3 = (struct msgbuf*)malloc(sizeof(*msger3));


	//set up signals
	signal_setup();

	//get shared memory
	id = shmget(key, sizeof(shm), IPC_CREAT | 0666);
	ptr = (shm*) shmat(id, NULL, 0);
	
	//get message queue
	msg = msgget(key, 0666 | IPC_CREAT);

	while(1){
		msgrcv(msg, msger, sizeof(msger), 1, IPC_NOWAIT);
		msgrcv(msg, msger2, sizeof(msger2), 2, IPC_NOWAIT);
		msgrcv(msg, msger3, sizeof(msger3), 3, IPC_NOWAIT);
		if(msger3->mtype == 3){
			msger3->num = mngid;
			msgsnd(msg, msger3, sizeof(msger3), 0);
		}

		for(counter = 0; counter <= 20; counter++){
			if(ptr->process[counter][0] == msger->num)
				break;
			if(ptr->process[counter][0] == 0){
				ptr->process[counter][0] = msger->num;
				break;
			} 
		}
		for(counter = 0; counter <= 20; counter++){
			if(ptr->perfect[counter] == msger2->num){
				check = false;
				break;
			}
		}
		if(check == true){
			ptr->perfect[perfcount] = msger2->num;
			perfcount++;
		}
		if(check == false)
			check = true;
	}
}

void signal_setup(){
	sigset_t mask;
	struct sigaction act;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGQUIT);
	act.sa_flags = 0;
	act.sa_mask = mask;
	act.sa_handler = signal_handler;

	sigaction(SIGINT, &act, NULL);
	sigaction(SIGHUP, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
}

void signal_handler(int signum){
	int i;
	for(i = 0; i < 20; i++)
		kill(ptr->process[i][0], SIGINT);
	sleep(5);
	shmdt(ptr);
	_exit(EXIT_FAILURE);
}