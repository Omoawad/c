/***
 * shm.c - A shared memory demo.
 *
 * Inspired by examples in Advanced Programming in the UNIX Environment, 
 * third edition, pp. 529.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>


void print_error(const char*, int);

sem_t mutex;

int main(int argc, char* argv[])
{

	sem_init(&mutex, 0 , 1);
	/*
	 * Get a key I can share with other processes.
	 */
	int tokid = 0;
	char *filepath = "/tmp";

	key_t key;
	if ((key = ftok(filepath, tokid)) == -1)
		print_error("Can not create token", errno);

	printf("Token id: %d\n", key);

	/*
	 * Get an id for the shared memory space.
	 */

	long bufsz = sysconf(_SC_PAGESIZE);
	printf("Page size: %ld\n", bufsz);

	int shmid;
	if ((shmid = shmget(key, bufsz, IPC_CREAT | 0666)) == -1)
		print_error("Can not create shared memory", errno);

	printf("Shared memory id: %d\n", shmid);

	/*
	 * Attach: Get a pointer to the memory space. 
	 */

	char *shm = NULL;
	if ((shm = shmat(shmid, 0, 0)) == (void *)-1)
		print_error("Unable to attach to shared memory", errno);

	/*
	 * Write to the shared memory.
	 */ 
	
	int shmlen = strlen(shm);
	printf("Shared memory bytes used: %d\n", shmlen);

	char *cbuf;
	char *name;
	
	size_t len = 0;
	ssize_t mesLen = 0;
	
	//Client
	if(argc > 1 && 0 == strcmp(argv[1], "exit")){
		//shm == NULL;
		memset(shm, '\0', sizeof(char));
		if(shmdt(shm) == -1){
			printf("error, cannot detach shared mem\n");
		}
		if(shmctl(shmid, IPC_RMID, 0) == -1){
			printf("Shared memory not valid\n");
		}
		exit(0);
	}else{
		while(1){
			int memlen = strlen(shm);
			
			printf("Enter name:\n");
			//scanf_s("%s\n" , &name, sizeof(stdin));
			mesLen = getline(&name, &len, stdin);
			while(1){
		//	printf(name);
				printf("New message from %s" , name, "!");
			
				mesLen = getline(&cbuf, &len, stdin);
				name[strlen(name) - 1] = '\0';
				strcat(name , ": ");
				strcat(name, cbuf); 
				sem_wait(&mutex);
				//cbuf == NULL;						
	
				int cbuflen = strlen(name);
				memlen = strlen(shm);
				if (shmlen + cbuflen + 1 < bufsz) {
					printf("Before write (%lu): %s\n", strlen(shm), shm);

					memcpy(shm + shmlen, cbuf,  cbuflen + 1);

					printf("After write (%lu): %s\n", strlen(shm), shm);
				}else {
					printf("Buffer full\n");
					memset(shm, '\0', sizeof(char));
					}
				sem_post(&mutex);
			
			}
		}
	}
	exit(0);
}

void print_error(const char* str, int code)
{
	printf("%s: %s\n",str, strerror(code));
	exit(-1);

}

//  END OF FILE