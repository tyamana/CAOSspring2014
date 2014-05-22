#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

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
		int i, j; //counters
		int fd, stdin_fd = 0;
		char* argv[MAXARG];
		for(i = 0; i < MAXARG; i++)
			argv[i] = (char*)malloc(MAXSIZE*sizeof(char));
		char* argument = (char*)malloc(MAXSIZE*sizeof(char)); 
		int flags[5] = {0};
		/*flags[o] - memory flag, if it == 1, then we should realloc()
		  flags[1] - output to a file flag 
		  flags[2] - output to an end of file flag 
		  flags[3] - run ib background flag
		  flags[4] - conveyor*/
		int fileIndex = 0; // index of argument in argv with path of file
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
					if( !flags[4] )
						flags[4] = 1;
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
			//run in background
			else if (c == '&' && i == 0) 
			{
				argument[i++] = c;
				read(0, buf, 1);
				c = buf[0];
				if(c == ' ' || c == '\n')
				{
					flags[3] = 1;
					i = 0;
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
			argv[j] = NULL;
			if(strcmp(argv[0], "pwd") == 0)
			{
				char curDir[50];
				getcwd(curDir, 50);
				printf("%s\n", curDir);
			}
			else if(strcmp(argv[0], "cd") == 0)
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
			else if (flags[4]) //CONVEYOR
			{
				int pipefd[2];
				int pipefderr[2]; // for errors
				int stdin_fd = dup(0);    //    for
				int stdout_fd = dup(1);   // return fd
				int prevout = dup(0);
				int oldstdout = dup(1);
				int children[MAXARG];
				int childrenerr[MAXARG];
				int i;
				
				for (i = 0; i <= delimsNum; ++i) 
				{	
					pipe(pipefd);
					pipe(pipefderr);
					if (i != delimsNum)
						argv[ convDelimsId[i] ] = NULL;
					int child1 = fork();	
					if (child1 == 0) 
					{
						close(pipefderr[0]);
						close(pipefd[0]);
						dup2(prevout, 0);
						dup2(pipefderr[1], 2);
						if ( i != (delimsNum) ) {
							dup2(pipefd[1], 1);
						}
						else {
							dup2(oldstdout, 1);			
						}			
						if (i > 0) 
							execvp(argv[ convDelimsId[i - 1] + 1 ], argv + convDelimsId[i - 1] + 1 );
						else
							execvp(argv[0], argv );
					}
					else 
					{							
						children[i] = child1;
						close(pipefderr[1]);
						close(pipefd[1]);
						int child2 = fork();
						if (child2 == 0) {
							close(pipefd[0]);
							int error_flag = 0;
							char c;
							while (read(pipefderr[0], &c, 1) == 1) {
								if (!error_flag) {
									fprintf(stderr, "%d: begin of error list\n", i);
									error_flag = 1;			
								}					
								fprintf(stderr, "%c", c);				
							}
							if (error_flag) {
								fprintf(stderr, "%d: end of error list\n", i);	
							}
							close(pipefderr[0]);
							return 0;
						}
						else {
							childrenerr[i] = child2;	
						}	
						close(pipefderr[0]);
						close(prevout);
						prevout = pipefd[0];	
					}
				}
				close(prevout);
				for (i = 0; i < delimsNum + 1; ++i) {
					waitpid(childrenerr[i], NULL, 0);	
				}
				for (i = 0; i < delimsNum + 1; ++i) {
					int status;
					waitpid(children[i], &status, 0);
				}
				dup2(stdin_fd, 0);
				dup2(stdout_fd, 1);
			}
			// RUNNING
			else if (flags[3]) // run in background
			{ 
				int PID = fork();	
				if (PID < 0) 
				{ 
					perror("FORK");
					continue;	
				}
				
				if (PID == 0)
				{ // потомок выполняет
					int stdin_fd = 0;
					if (flags[1]) {
						stdin_fd = dup(1);
						int fd = open(argv[fileIndex], O_CREAT | O_TRUNC | O_WRONLY, 0777);
						if (fd < 0)
						{
							perror("> FILE");
							continue;
						}
						if (dup2(fd, 1) < 0) {
							perror("DUP");
							continue;
						}
					}
					else if (flags[2]) {
						stdin_fd = dup(1);
						int fd = open(argv[fileIndex], O_APPEND | O_WRONLY, 0777);
						if (fd < 0) {
							perror(">> FILE");
							continue;
						}
						if (dup2(fd, 1) < 0) {
							perror("DUP");
							continue;
						}
					}
					if (execvp(argv[0], argv) == -1) 
					{ // запускаем процесс
						perror("EXEC");
						continue;
					}
					close(fd);
					if(stdin_fd != 0)
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
				int PID = fork();	
				if (PID < 0) 
				{ 
					perror("FORK");
					continue;	
				}
				
				if (PID == 0)
				{ // потомок выполняет
					int stdin_fd = dup(1);
					if (flags[1]) {
						int fd = open(argv[fileIndex], O_CREAT | O_TRUNC | O_WRONLY, 0777);
						if (fd < 0)
						{
							perror("> FILE");
							continue;
						}
						if (dup2(fd, 1) < 0) {
							perror("DUP");
							continue;
						}
					}
					else if (flags[2]) {
						int fd = open(argv[fileIndex], O_APPEND | O_WRONLY, 0777);
						if (fd < 0) {
							perror(">> FILE");
							continue;
						}
						if (dup2(fd, 1) < 0) {
							perror("DUP");
							continue;
						}
					}
					if (execvp(argv[0], argv) == -1) 
					{ // запускаем процесс
						perror("EXEC");
						continue;
					}
					close(fd);
					dup2(stdin_fd, 1);
					return;
				}
				else 
				{ 
					int status;
					waitpid(PID, &status, WUNTRACED);
				}
			}
		}
	}
		
	return 0;
}
