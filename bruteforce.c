#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>

pthread_t THREADS[10]; 
long int base;
long int module;
long int result;
long int secret;

void SecretIsFound(long int i)
{
	pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
	if(pthread_mutex_trylock(&m) == 0);
	{
		secret = i;
		pthread_mutex_unlock(&m);
	}
	for(i = 0; i < 10; i++)
		pthread_cancel(THREADS[i]);
	pthread_mutex_destroy(&m);
}

void* SecretFinder(void* p)
{
	long int i = *(long int*)p;
	//borders of working for each thread
	long int from = 10000000*i + 1;
	long int to = 10000000*(i + 1);
	//long int from = i * (long int)sqrt((double)LONG_MAX) / 10 + 1;
	//long int to = (i + 1)*(long int)sqrt((double)LONG_MAX) / 10;
	
	//fast exponentiation
	i = from;
	long int a = base;
	long int res = 1;
	while( i > 0 )
	{
		if (i & 1 == 1) res *= a;
		i >>= 1;
		a *= a;
	}
	
	res %= module;
	if(res == result)
		SecretIsFound(i);
	for(i = from + 1; i <= to; i++)
	{
		res *= base;
		res %= module;
		if(res == result)
		{
			SecretIsFound(i);
			return p;
		}
	}
	
	return p;
}

void bruteforce()
{
	long int i;
	long int buf[10];
	for(i = 0; i < 10; i++)
		buf[i] = i;
	for(i = 0; i < 10; i++)
	{
		if(pthread_create(THREADS + i, NULL, SecretFinder, buf+i))
		{
			write(1, "pthread_create() error\n", 21);
			_exit(1);
		}
	}	
	for(i = 0; i < 10; i++)
		pthread_join(THREADS[i], NULL);
}

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		write(1, "wrong number of parameters\n", 27);
		_exit(1);
	}
	
	base = atol(argv[1]);
	module = atol(argv[2]);
	result = atol(argv[3]);
	if(base <= 0 || module <= 0 || result <= 0)
	{
		write(1, "wrong input\n", 12);
		_exit(2);
	}
	bruteforce();
	printf("%ld\n", secret);
	
	return 0;
}
