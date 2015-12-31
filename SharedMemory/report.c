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

int main(int argc, char *argv[]){
	int i, k, id, msg;
	int tested = 0;
	key_t key = 85108;
	msger = (struct msgbuf*)malloc(sizeof(*msger));

	//set up signals
	signal_setup();

	//create shared memory
	id = shmget(key, sizeof(shm), IPC_CREAT | 0666);	
	ptr = (shm*) shmat(id, NULL, 0);

	//get message queue
	msg = msgget(key, 0666 | IPC_CREAT);

	//-k signals manager to shut down
	if(argc == 2){
		if(strcmp(argv[1], "-k") == 0){
			msger->mtype = 3;
			msgsnd(msg, msger, sizeof(msger), 0);
			msgrcv(msg, msger, sizeof(msger), 3, 0);
			kill(msger->num, SIGINT);
		}
	}

	//count total number tested
	for(i = 0; i < 8000; i++){
		for(k = 0; k < 32; k++){
			unsigned int flag = 1;
			flag = flag << k;
			if(ptr->bits[i] & flag){
				tested++;
			}
			else
				continue;
		}
	}

	printf("Perfect numbers found:\n ");
	for(i = 0; i < 20; i++){
		if(ptr->perfect[i] == 0)
			break;
		printf("%d ", ptr->perfect[i]);
	}
	printf("\nTotal number tested: %d\n ", tested);
	for(i = 0; i < 20; i++){
		if(ptr->process[i][0] == 0)
			break;
		printf("Compute process %d: tested:%d skipped:%d found:%d\n", ptr->process[i][0], ptr->process[i][2], ptr->process[i][3], ptr->process[i][1]);
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
	_exit(EXIT_FAILURE);
}