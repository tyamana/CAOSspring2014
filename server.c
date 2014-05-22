#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define ERR_SOCKET -1
#define ERR_BIND -2
#define ERR_LISTEN -3
#define ERR_SELECT -4
#define ERR_ACCEPT -5
#define ERR_READ -6

fd_set fds;
int fdID[100];
char names[100][20];
int fdNum = 0;
int maxFD = 0; //for using in select()
struct sockaddr_in sa;
int sock;

void PrintError(char* s, int errNum)
{
	close(sock);
	perror(s);
	_exit(errNum);
}

int max(int a, int b)
{
	return ( (a > b) ? a : b ); 
}

int main()
{
	memset(&sa, 0, sizeof(sa)); 
	sa.sin_family = PF_INET;
	sa.sin_port = htons(4400);
	sa.sin_addr.s_addr = INADDR_ANY;

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
		PrintError("socket() error", ERR_SOCKET);
	if(bind(sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
		PrintError("bind() error", ERR_BIND);
	if(listen(sock, 10) < 0)
		PrintError("listen() error", ERR_LISTEN);
	
	write(1, "Server is working\n", 18);
	write(1, "To turn it off enter \"quit\"\n", 28);
		
	fd_set rdset, wrset;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	FD_SET(sock, &fds);
	maxFD = sock;
	while(1)
	{
		FD_ZERO(&wrset);
		FD_ZERO(&rdset);
		memcpy(&rdset, &fds, sizeof(fds));
		int n = select(maxFD + 1, &rdset, NULL, NULL, NULL);
		if(n < 0)
			PrintError("select() error", ERR_SELECT);
		else
		{ 
			char buf[1024] = {0};
			if(FD_ISSET(0, &rdset))
			{
				int bc = read(0, buf, 5);
				if(bc == 5 && strcmp(buf, "quit\n") == 0)
				{
					close(sock);
					return 0;
				}
			}
			if(FD_ISSET(sock, &rdset))
			{
				struct sockaddr saddr;
				socklen_t sl;
				int cfd = accept(sock, &saddr, &sl);
				if(cfd < 0)
					PrintError("accept() error", ERR_ACCEPT);
				FD_SET(cfd, &fds);
				fdID[fdNum] = cfd;
				sprintf(names[fdNum], "username%d", fdNum);
				write(cfd, "Your nickname: ", 15);
				write(cfd, names[fdNum], strlen(names[fdNum]));
				write(cfd, "\n\0", 2);
				fdNum++;
				maxFD = max(maxFD, cfd);
				n--;
			}
			int i = 0;
			while(n > 0)
			{
				if(FD_ISSET(fdID[i], &rdset))
				{
					int bc = read(fdID[i], buf, 1024); //byte counter
					if(bc < 0)
						PrintError("read() error", ERR_READ);
					if(bc == 0)
					{ 
						FD_CLR(fdID[i], &fds);
						fdID[i] = -1;
					}
					else
					{
						int j;
						for(j = 0; j < fdNum; j++)
							if( j != i && fdID[j] != -1)
							{
								write(fdID[j], names[i], strlen(names[i]));
								write(fdID[j], ": ", 2);
								write(fdID[j], buf, bc);
							}
					}
					n--;
				}
				i++;
			}
		}
	}
		
	close(sock);
	return 0;                                                                                                                           
}








