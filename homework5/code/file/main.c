//
// Created by bear on 12/3/2022.
//
#include "apue.h"
#include "lock.h"
#include "error.h"

#include "sys/wait.h"

#define N 5

static char *forks[5] = {"fork0", "fork1", "fork2", "fork3", "fork4"};

void putFork(int i)
{
	if (i == N - 1)
	{
		unlock(forks[0]);
		unlock(forks[i]);
	}
	else
	{
		unlock(forks[i]);
		unlock(forks[i + 1]);
	}
}

void takeFork(int i)
{
	if (i == N - 1)
	{
		lock(forks[0]);
		lock(forks[i]);
	}
	else
	{
		lock(forks[i]);
		lock(forks[i + 1]);
	}
}

void eating(int i, int nsecs)
{
	printf("philosopher %d is eating\n", i);
	sleep(nsecs);
}

void thinking(int i, int nsecs)
{
	printf("philosopher %d is thinking\n", i);
	sleep(nsecs);
}

void philosopher(int i, int nsecs)
{
	for (;;)
	{
		thinking(i, nsecs);
		takeFork(i);
		eating(i, nsecs);
		putFork(i);
	}
}

int main(int argc, char **argv)
{
	int nsecs = 0;
	if (argc != 3 && argc != 1)
	{
		printf("usage: %s [-t <nsecs>]", argv[0]);
	}

	if (strcmp(argv[2], "-t") == 0)
	{
		nsecs = atoi(argv[3]);
	}
	else
	{
		nsecs = 2;
	}

	for (int i = 0; i < N; i++)
	{
		pid_t pid = fork();
		if (pid == 0)
		{
			philosopher(i, nsecs);
		}
	}

	wait(NULL);
	return 0;
}