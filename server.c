#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define ERRSOCKET -1
#define ERRBIND -2
#define ERRLISTEN -3
#define ERRSELECT -4
#define ERRACCEPT -5

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
fd_set fds;
int fdID[100];
char names[100][20];
int fdNum = 0;
int maxFD = 0; //for using in select()
struct sockaddr_in sa;
int sock;
pthread_t threads[2];

void PrintError(char* s, int errNum)
{
	close(sock);
	pthread_mutex_destroy(&m);
	perror(s);
	_exit(errNum);
}

int max(int a, int b)
{
	if(a > b)
		return a;
	else
		return b;
}

void* Accepting(void* p)
{
	FD_ZERO(&fds);
	write(1, "Server is working\n", 18);
	write(1, "To turn it off enter \"quit\"\n", 28);
	while(1)
	{
		struct sockaddr saddr;
		socklen_t sl;
		int cfd = accept(sock, &saddr, &sl);
		if(cfd < 0)
			PrintError("accept() error", -5); 
		pthread_mutex_lock(&m);
		FD_SET(cfd, &fds);
		fdID[fdNum] = cfd;
		sprintf(names[fdNum], "username%d", fdNum);
		write(cfd, "Your nickname: ", 15);
		write(cfd, names[fdNum], strlen(names[fdNum]));
		write(cfd, "\n\0", 2);
		fdNum++;
		maxFD = max(maxFD, cfd);
		pthread_mutex_unlock(&m);
	}
}

void* ReadWrite(void* p)
{
	struct timeval t;
	t.tv_sec = 30;
	t.tv_usec = 0;
	fd_set rdset, wrset;
	while(1)
	{
		pthread_mutex_lock(&m);
		FD_ZERO(&rdset);
		memcpy(&rdset, &fds, sizeof(fds));
		FD_SET(0,&rdset);
		int n = select(maxFD + 1, &rdset, NULL, NULL, &t);
		if(n < 0)
			PrintError("select() error", ERRSELECT);
		if(n > 0)
		{
			char buf[1024] = {0};
			if(FD_ISSET(0, &rdset))
			{
				int bc = read(0, buf, 5);
				if(bc == 5 && strcmp(buf, "quit\n") == 0)
					pthread_cancel(threads[1]);
			}
			int i = 0;
			do
			{
				if(FD_ISSET(fdID[i], &rdset))
				{
					int bc = read(fdID[i], buf, 1024); //byte counter
					FD_ZERO(&wrset);
					memcpy(&wrset, &fds, sizeof(fds));
					int m = select(maxFD + 1, NULL, &wrset, NULL, NULL);
					if(m < 0)
						PrintError("select() error", ERRSELECT);
					int j = 0;
					m--; //want to send a message to all clients except this
					do
					{
						if( j != i && FD_ISSET(fdID[j], &wrset))
						{
							write(fdID[j], names[i], strlen(names[i]));
							write(fdID[j], ": ", 2);
							write(fdID[j], buf, bc);
							m--;
						}
						j++;
					} while(m > 0);
					n--;
				}
				i++;
			} while(n > 0);
		}
		pthread_mutex_unlock(&m);
	}
}

int main()
{
	memset(&sa, 0, sizeof(sa)); 
	sa.sin_family = PF_INET;
	sa.sin_port = htons(4400);
	sa.sin_addr.s_addr = INADDR_ANY;
	
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
		PrintError("socket() error", ERRSOCKET);
	if(bind(sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
		PrintError("bind() error", ERRBIND);
	if(listen(sock, 10) < 0)
		PrintError("listen() error", ERRLISTEN);
	
	pthread_create(threads, NULL, Accepting, NULL);
	pthread_create(threads + 1, NULL, ReadWrite, NULL);
	pthread_join(threads[1], NULL);
	
	pthread_cancel(threads[0]);
	close(sock);
	pthread_mutex_destroy(&m);
	return 0;                                                                                                                           
}
