//
// Created by bear on 12/3/2022.
//
#include "apue.h"
#include "semaphore.h"
#include "error.h"

#include "sys/wait.h"
#include "sys/fcntl.h"

#define N 5

static sem_t forks_sems[5];

void init_sems()
{
	for (int i = 0; i < N; i++)
	{
		if (sem_init(&forks_sems[i], 1, 1) != 0)
		{
			err_sys("sem_open error");
		}
	}
}

void destroy_sems()
{
	for (int i = 0; i < N; i++)
	{
		sem_destroy(&forks_sems[i]);
	}
}

void putFork(int i)
{
	if (i == N - 1)
	{
		sem_post(&forks_sems[0]);
		sem_post(&forks_sems[i]);
	}
	else
	{
		sem_post(&forks_sems[i]);
		sem_post(&forks_sems[i + 1]);
	}
}

void takeFork(int i)
{
	if (i == N - 1)
	{
		sem_wait(&forks_sems[0]);
		sem_wait(&forks_sems[i]);
	}
	else
	{
		sem_wait(&forks_sems[i]);
		sem_wait(&forks_sems[i + 1]);
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

	init_sems();

	for (int i = 0; i < N; i++)
	{
		pid_t pid = fork();
		if (pid == 0)
		{
			philosopher(i, nsecs);
		}
	}

	wait(NULL);
	destroy_sems();
	return 0;
}