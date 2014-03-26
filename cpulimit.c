#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

void SIGCHLDcatcher(int a) { exit(0); }

int cpulimit(long int limit, char** argv)
{
	int pid = fork();
	if(pid == 0)
	{
		if(execvp(argv[2], argv + 2) == -1)
		{
			perror("execvp( ) error ");
			exit(-3);
		}
	}
	else if(pid > 0)
	{
		while(1)
		{
			kill(pid, SIGSTOP);
			if(usleep((100 - limit)*10000))
			{
				perror("usleep( ) error ");
				exit(-1);
			}
			kill(pid, SIGCONT);
			if(usleep(limit*10000))
			{
				perror("usleep( ) error ");
				exit(-1);
			}
		}
	}
	else 
	{
		perror("fork() error ");
		exit(-2);
	}
}

int main(int argc, char** argv)
{	
	if(argc <= 3)
	{
		write(1, "Not enough parameters\n", 27);
		return -1; 
	}
	long int percentage = atol(argv[1]);
	if(percentage <= 0 || 100 <= percentage)
	{
		write(1, "Wrong limit parameter\n", 22);
		return -2;
	}
	
	struct sigaction act;
	act.sa_handler = SIGCHLDcatcher;
	act.sa_flags = SA_NOCLDSTOP;
	if(sigaction(SIGCHLD, &act, NULL) == -1)
	{
		perror("sigaction( ) error ");
		return -3;
	}
	
	cpulimit(percentage, argv);
	
	return 0;
}
