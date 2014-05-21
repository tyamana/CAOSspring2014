#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ERRSIGACT -1

int MAXARG = 10;
int MAXSIZE = 100;

void SigintCatcher(int a)
{
	write(1,"\n", 1);
	_exit(0);
}

void PrintError(char* s, int errNum)
{
	perror(s);
	_exit(errNum);
}

int main()
{
	// ctrl+c catcher
	struct sigaction act;
	act.sa_handler = SigintCatcher;
	if(sigaction(SIGINT, &act, NULL) == -1)
		PrintError("sigaction() error", ERRSIGACT);
	
	while(1)
	{
		//write(1, "external\n", 9);
		int i, j; //counters
		int fd, stdin_fd = 0;
		char* argv[MAXARG];
		for(i = 0; i < MAXARG; i++)
			argv[i] = (char*)malloc(MAXSIZE*sizeof(char));
		char* argument = (char*)malloc(MAXSIZE*sizeof(char)); 
		int flags[3] = {0, 0, 0};
		/*flags[o] - memory flag, if it == 1, then we should realloc()
		  flags[1] - output to a file flag 
		  flags[2] - output to an end of file flag */
		int run_in = 0; // run in background
		int fileIndex = 0; // index of argument in argv with path of file
		int conveyorWasDetected = 0;
		int convDelimsId[100] = {0};
		int delimsNum = 0; //number of delimiters
		i = j = 0; // j = counter of number of arguments
				   // i = counter of length of argument
		char c; //new character from stream
		char buf[1];
		int argumentsAreCorrect = 1;
		int lastArgumentHasRead = 0;
		while( !lastArgumentHasRead )
		{
			//write(1, "internal\n", 9);
			read(0, buf, 1);
			c = buf[0];
			//conveyor catcher
			if(c == '|' && i == 0)
			{
				argument[i++] = c;
				read(0, buf, 1);
				c = buf[0];
				if(c == ' ')
				{
					argument[i] = '\0';
					strcpy(argv[j], argument);
					convDelimsId[delimsNum] = j;
					j++;
					delimsNum++;
					i = 0;
					if( !conveyorWasDetected )
						conveyorWasDetected = 1;
				}
				else
				{
					while( c != ' ' && c != '\n')
					{
						argument[i] = c;
						read(0, buf, 1);
						c = buf[0];
						i++;
					}
					argument[i] = '\0';
					printf("Unknown command \"%s\"\n", argument);
					argumentsAreCorrect = 0;
				}
			}
			else if (c == '&') 
			{
				run_in == 1;
			}
			//output in file
			else if(c == '>' && i == 0)
			{
				argument[i++] = c;
				read(0, buf, 1);
				c = buf[0];
				if(c == '>')
				{
					argument[i] = c;
					i++;
					read(0, buf, 1);
					c = buf[0];
					if(c == ' ')
					{
						argv[j] = NULL;
						j++;
						i = 0;
						fileIndex = j;
						flags[2] = 1;
					}
					else
						argumentsAreCorrect = 0;
				}
				else if(c == ' ')
				{
					argv[j] = NULL;
					j++;
					i = 0;
					fileIndex = j;
					flags[1] = 1;
				}
				else
					argumentsAreCorrect = 0;
				
				if( !argumentsAreCorrect )
				{
					while( c != ' ' && c != '\n')
					{
						argument[i] = c;
						read(0, buf, 1);
						c = buf[0];
						i++;
					}
					argument[i] = '\0';
					printf("Unknown command \"%s\"\n", argument);
				}
			}
			//input from file
			else if(c == '<' && i == 0)
			{
				read(0, buf, 1);
				c = buf[0];
				if(c != ' ')
				{
					while( c != ' ' && c != '\n')
					{
						argument[i] = c;
						read(0, buf, 1);
						c = buf[0];
						i++;
					}
					argument[i] = '\0';
					printf("Unknown command \"<%s\"\n", argument);
					argumentsAreCorrect = 0;
				}
				else
				{
					read(0, buf, 1);
					c = buf[0];
					while( c != ' ' && c != '\n')
					{
						argument[i] = c;
						i++;
						if(i == MAXSIZE - 1)
						{
							argument = (char*)realloc(argument, 2*MAXSIZE*sizeof(char));
							flags[0] = 1;
							MAXSIZE *= 2;
						}
						read(0, buf, 1);
						c = buf[0];
					}
					argument[i] = '\0';
					stdin_fd = dup(0);
					fd = open(argument, O_RDONLY);
					if(fd < 0)
					{
						perror(NULL);
						argumentsAreCorrect = 0;
					}
					else
						dup2(fd,0);	
					i = 0;
					c = 0; //to predict a finish of cycle
				}
				
			}
			else if(c != ' ' && c != '\n')
			{
				argument[i] = c;
				i++;
				if(i == MAXSIZE - 1)
				{
					argument = (char*)realloc(argument, 2*MAXSIZE*sizeof(char));
					flags[0] = 1;
					MAXSIZE *= 2;
				}
			}
			else if(i != 0)
			{
				if(flags[0])
				{
					argv[j] = (char*)realloc(argv[j], MAXSIZE*sizeof(char));
					flags[0] = 0;
				}
				argument[i] = '\0';
				strcpy(argv[j], argument);
				j++;
				i = 0;
			}
			if(c == '\n')
				lastArgumentHasRead = 1;
		}
		
		if(stdin_fd != 0)
		{
			close(fd);
			dup2(stdin_fd, 0);
		}
			
		if(argumentsAreCorrect)
		{
			//write(1, "ya tut\n", 7);
			//for(i = 0; i < j; i++)
			//	printf("%s\n", argv[i]);
			
			if(strcmp(argv[0], "pwd") == 0)
			{
				char curDir[50];
				getcwd(curDir, 50);
				printf("%s\n", curDir);
			}
			
			if(strcmp(argv[0], "cd") == 0)
			{
				char curDir[50];
				getcwd(curDir, 50);
				if(j == 1 || !strcmp(argv[1], ">") || !strcmp(argv[1], "|")
				|| !strcmp(argv[1], ">>") || !strcmp(argv[1], "<"))
				{
					char* homeDir = getenv("HOME");
					if(chdir(homeDir) < 0)
						perror(NULL);
					else 
						printf("%s\n", homeDir);
				}
				else
				{
					strcat(curDir, "/");
					strcat(curDir, argv[1]);
					if(chdir(curDir) < 0)
						perror(NULL);
					else 
						printf("%s\n", curDir);
				}
			}
		}
		// RUNNING
		if (run_in) // run in background
		{ 
		
			int PID = fork();	
			if (PID < 0) 
			{ 
				perror("FORK");
				exit(0);	
			}
			
			if (PID == 0)
			{ // потомок выполняет
				int stdin_fd = dup(1);
				if (flags[1]) {
					int fd = open(argument[fileIndex], O_CREATE | O_TRUNC, 0777);
					if (fd < 0)
					{
						perror("> FILE");
						continue;
					}
					if (dup2(fd, 1) < 0) {
						perror("DUP");
					}
				}
				else if (flags[2]) {
					int fd = open(argument[fileIndex], O_APPEND, 0777);
					if (fd < 0) {
						perror(">> FILE");
						continue;
					}
					if (dup2(fd, 1) < 0) {
						perror("DUP");
					}
				}
				if (execvp(argv[0], argv) == -1) 
				{ // запускаем процесс
					perror("EXEC");
					return;
				}
				close(fd);
				dup2(stdin_fd, 1);
				return;
			}
			else 
			{ // родитель читает дальше
				continue;
			}	
		}
		else 
		{
			int stdin_fd = dup(1);
			if (flags[1]) {
				int fd = open(argument[fileIndex], O_CREATE | O_TRUNC, 0777);
				if (fd < 0)
				{
					perror("> FILE");
					continue;
				}
				if (dup2(fd, 1) < 0) {
					perror("DUP");
				}
				
			}
			else if (flags[2]) {
				int fd = open(argument[fileIndex], O_APPEND, 0777);
				if (fd < 0) {
					perror(">> FILE");
					continue;
				}
				if (dup2(fd, 1) < 0) {
					perror("DUP");
				}
			}
			if (execvp(argv[0], argv) == -1) 
			{ // запускаем процесс
				perror("EXEC");
				return;
			}
			close(fd);
			dup2(stdin_fd, 1);
		}

	}
		
	return 0;
}
