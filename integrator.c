#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long double func(long double x)
{
	return (x*x);
}

long double RectangleMethod(long double a, long double b, long double dx, long double(*f)(long double))
{
	long double x = a, area = 0.;
	while(x < b)
	{
		area += dx*f(x);
		x += dx;
	}
	return area;
}

long double Integrator(long double (*f)(long double), long double a, long double b, long double dx, int workersNum)
{
	int pipefd[2];
	if(pipe(pipefd))
	{
		perror("Pipe creation error");
		exit(-1);
	}
	
	int i = 0;
	for(; i < workersNum; i++)
	{
		int pid = fork();
		if (pid == 0)
		{
			close(pipefd[0]);
			long double area = RectangleMethod(a + (b - a)/workersNum*i,
											   a + (b - a)/workersNum*(i + 1),
											   dx,
											   func);
			write(pipefd[1], &area, sizeof(long double));
			close(pipefd[1]);
			exit(0);
		}
		else if(pid < 0)
			perror("Fork error");
	}
	
	close(pipefd[1]);
	long double areas[workersNum];
	
	for(i = 0; i < workersNum; i++)
		read(pipefd[0], &areas[i], sizeof(long double));
	close(pipefd[0]);
	
	long double integral = 0;
	for(i = 0; i < workersNum; i++)
		integral += areas[i];
	return integral;
}
int main()
{
	double a = 0., b = 10., dx = 0.000001;
	int workersNum = 10;
	char buf[sizeof(long double)];
	int n = sprintf(buf, "%Lf\n", Integrator(func, a, b, dx, workersNum));
	write(1, buf, n);
	
	return 0;
}
