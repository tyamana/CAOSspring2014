#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	if(argc >= 3)
	{
		long int begin = atol(argv[1]);
		long int end = atol(argv[2]);
		for(;begin <= end; begin++)
		{
			printf("%ld ", begin);
			if(begin % 10 == 0)
				printf("\n");
		}
	}
	
	return 0;
}

