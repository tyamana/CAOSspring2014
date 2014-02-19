#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		write(2, "Wrong number of parameters\n", 27);
		_exit(1); 
	}
	
	int file1 = open(argv[1], O_RDONLY);
	if (file1 < 0)
	{
		perror(argv[0]);
		_exit(1);
	}
	int file2 = open(argv[2], O_WRONLY);
	if (file2 < 0)
	{
		perror(argv[0]);
		_exit(1);
	}
	
	int n, byte_counter = 0;
	char buf[1000];
	do
	{
		n = read(file1, buf + byte_counter, 1);
		if (n != 0)
			byte_counter++;
		if (n < 0)
		{
			perror(argv[0]);
			_exit(1);
		}	
	}
	while(n > 0);
	
	n = write(file2, buf, byte_counter);
	if (n < 0)
	{
		perror(argv[0]);
		_exit(1);
	}
	
	close(file1);
	close(file2);
	_exit(0);
}
