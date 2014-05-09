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
#define ERRCONNECT -2
#define ERRSELECT -3

void PrintError(char* s, int errNum)
{
	perror(s);
	_exit(errNum);
}

int main(int argc, char** argv)
{
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = PF_INET;
	sa.sin_port = htons(4400);
	sa.sin_addr.s_addr = INADDR_ANY;
	
	int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
		PrintError("socket() error", ERRSOCKET);
	if(connect(sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
		PrintError("connect() error", ERRCONNECT);
		
	write(1, "Connected to server\n", 20);
		
	////Non-blocking input
	//int flags = fcntl(sock, F_GETFL, 0);
	//if (flags == -1) flags = 0;
	//fcntl(sock, F_SETFL, flags | O_NONBLOCK);
	
	char buf[1024];
	while(1)
	{
		fd_set rdset;
		FD_ZERO(&rdset);
		FD_SET(0, &rdset);
		FD_SET(sock, &rdset);
		
		int n = select(sock + 1, &rdset, NULL, NULL, NULL);
		if(n < 0)
			PrintError("select() error", ERRSELECT);
		if(n > 0)
		{
			if(FD_ISSET(sock, &rdset))
			{
				int bc = read(sock, buf, 1024);
				write(1, buf, bc);
			}
			if(FD_ISSET(0, &rdset))
			{
				int bc = read(0, buf, 1024);
				fd_set setForSock;
				FD_SET(sock, &setForSock);
				if(select(sock, NULL, &setForSock, NULL, NULL) < 0) 
					PrintError("select() error", ERRSELECT);
				write(sock, buf, bc);
			}
		}
	}
		
	close(sock);
	
	return 0;
}
