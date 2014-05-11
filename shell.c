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
		int i, j; //counters
		char* argv[MAXARG];
		for(i = 0; i < MAXARG; i++)
			argv[i] = (char*)malloc(MAXSIZE*sizeof(char));
		char* argument = (char*)malloc(MAXSIZE*sizeof(char)); 
		int flags[2] = {0, 0};
		/*flag[o] - memory flag, if it == 1, then we should realloc()
		  flag[1] - output to a file flag */
		int fileIndex = 0; // index of argument in argv with path of file
		
		i = j = 0; // j = counter of number of arguments
				   // i = counter of length of argument
		char c; //new character from stream
		int lastArgumentHasRead = 0;
		while( !lastArgumentHasRead )
		{
			c = fgetc(stdin);
			if(c == '>')
			{
				argument[i] = c;
				argument[i + 1] = '\0';
				strcpy(argv[j], argument);
				j++;
				i = 0;
				fileIndex = j;
				flags[1] = 1;
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
			if(j == 1 || !strcmp(argv[1], ">") || !strcmp(argv[1], "|"))
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
		
	return 0;
}
