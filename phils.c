#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NAME_LENGTH 20 //philosopher name
#define WORK_TIME 30   //time of program working in seconds

int file_fd;
int philNum;   //number of philosophers
char** phils; //names of philosophers 
pthread_mutex_t* forks;
pthread_mutex_t screen = PTHREAD_MUTEX_INITIALIZER;

void PrintLogString(char* s, int id)
{
	time_t timer;
	char logString[70] = {0};
	pthread_mutex_lock(&screen);
	time(&timer);
	memcpy(logString, ctime(&timer) + 11, 8);
	strcat(logString, " ");
	strcat(logString, phils[id]);
	strcat(logString, s);
	write(file_fd, logString, strlen(logString));
	pthread_mutex_unlock(&screen);
}

void Eat(int id)
{
	int f[2], ration, i;
	//forks
	f[0] = f[1] = id;
	//changing order of getting forks - key role in the program
	f[id & 1] = (id + 1) % philNum; 
	
	PrintLogString(" need forks!\n", id);
	
	for(i = 0; i < 2; i++)
	{
		pthread_mutex_lock(forks + f[i]);
		char s[30];
		sprintf(s," has got the fork%d\n",f[i]);
		PrintLogString(s, id);
		//a second delay for showing the strict order of getting forks
		sleep(1);
	}
	
	PrintLogString(" starts eating\n", id);
	for(i = 0, ration = 3 + rand() % 8; i < ration; i++)
		sleep(1);
	PrintLogString(" finishes eating\n", id);
	
	for(i = 0; i < 2; i++)
		pthread_mutex_unlock(forks + f[i]);
}

void Think(int id)
{
	PrintLogString(" starts thinking\n", id);
	int i, t;
	do
	{
		t = rand() % 5;
		usleep(1500000 + rand() % 1000000);
	} while(t);
	PrintLogString(" finishes thinking\n", id);
}

void* Philosophize(void* a)
{
	int id = *(int*)a;
	while(1)
	{
		Think(id);
		Eat(id);
	}
}

int main(int argc, char** argv)
{	
	if (argc != 2)
	{
		write(1, "wrong number of arguments\n", 26);
		_exit(1);
	}
	srand(time(NULL));
	file_fd = open("LOG.txt", O_WRONLY | O_TRUNC);
	if(file_fd < 0)
	{
		perror("open() error");
		_exit(2);
	}
	
	//number of philosophers
	philNum = atoi(argv[1]);
	//names of philosophers 
	phils = (char**)malloc(philNum*sizeof(char*));
	int i;
	for(i = 0; i < philNum; i++)
	{
		phils[i] = (char*)malloc(NAME_LENGTH*sizeof(char));
		sprintf(phils[i],"philosopher%d",i + 1);
	}
	//mutexes for forks
	forks = (pthread_mutex_t*)malloc(philNum*sizeof(pthread_mutex_t));
	
	int* id = (int*)malloc(philNum*sizeof(int));
	pthread_t* tid = (pthread_t*)malloc(philNum*sizeof(pthread_t));
	for(i = 0; i < philNum; i++)
		pthread_mutex_init(forks + (id[i] = i), NULL);
	for(i = 0; i < philNum; i++)
		pthread_create(tid + i, NULL, Philosophize, id + i);
	
	sleep(WORK_TIME);
	
	for(i = 0; i < philNum; i++)
		pthread_cancel(tid[i]);
	for(i = 0; i < philNum; i++)	
		pthread_mutex_destroy(forks + i);
	free(tid);
	free(id);
	for(i = 0; i < philNum; i++)
		free(phils[i]);
	free(phils);
	close(file_fd);
	pthread_mutex_destroy(&screen);
	
	return 0;
}
