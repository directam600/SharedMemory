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

bool findPerfect();
void signal_setup();
void signal_handler();
bool testable(int num);
void setBit(int num);

int location;
shm *ptr;
struct msgbuf *msger;

int main(int argc, char *argv[]){
	int N, id, msg, pid;	//msg used for pid, msg2 used for perfect numbers
	int i = 0;
	int pnfound = 0;
	int tested = 0;
	int ntestable = 0;
	key_t key = 85108;
	msger = (struct msgbuf*)malloc(sizeof(*msger));

	if(argc < 2){
		printf("No starting number entered\n");
		exit(EXIT_FAILURE);
	}
	sscanf(argv[1], "%d", &N);

	//set up signals
	signal_setup();

	//get shared memory
	id = shmget(key, sizeof(shm), IPC_CREAT | 0666);	
	ptr = (shm*) shmat(id, NULL, 0);

	//get message queue
	msg = msgget(key, 0666 | IPC_CREAT);

	pid = getpid();
	msger->mtype = 1;
	msger->num = pid;
	msgsnd(msg, msger, sizeof(msger), 0);

	for(i = 0; i < 20; i++){
		if(pid == ptr->process[i][0]){
			location = i;
			ptr->process[location][1] = pnfound;
			ptr->process[location][2] = tested;
			ptr->process[location][3] = ntestable;
		}
	}

	while(N <= 256000){  
		if(testable(N) == true){
			if(findPerfect(N) == true){
				pnfound++;
				ptr->process[location][1] = pnfound;
				msger->mtype = 2;
				msger->num = N;
				msgsnd(msg, msger, sizeof(msger), 0);
				printf("sent perfect number %d to message queue\n", msger->num);
			}
			setBit(N);		
			tested++;
		}
		else
			ntestable++;
		ptr->process[location][2] = tested;
		ptr->process[location][3] = ntestable;
		N++;
	}
}

bool findPerfect(int num){
	int i;
	int divisors = 1;
	for(i = 2; i < num; i++){
		if(num % i == 0)
			divisors += i;
	}
	if(divisors == num && divisors != 1){
		return true;
	}
	return false;
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
	for(i = 0; i < 4; i++)
		ptr->process[location][i] = 0;
	shmdt(ptr);		
	_exit(EXIT_FAILURE);
}

bool testable(int num){
	int bitnum;
	int intnum;
	unsigned int flag = 1;
	intnum = (num - 1) / 32;
	bitnum = (num - 1) % 32;	
	flag = flag << bitnum;
	if(ptr->bits[intnum] & flag){
		return false;
	}
	else 
		return true;
}

void setBit(int num){	//number 32 returns as bit 31.  33 returns as bit 0. 1 returns as bit 0
	int bitnum;
	int intnum;
	intnum = (num - 1) / 32;
	bitnum = (num - 1) % 32;
	ptr->bits[intnum] = ptr->bits[intnum] | (1 << bitnum);
}