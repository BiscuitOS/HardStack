// SPDX-License-Identifier: GPL-2.0
/*
 * Out-of-Order Execution at Compile Time
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

int flags[2];
int data;

void *thread_f(void *rank)
{
	long rankA = (long)rank;
	long rankB = 1 - rankA;

	while (1) {
		flags[rankA] = 1;
		__sync_synchronize(); /* Memory Barrier */
		data = rankB;
		__sync_synchronize(); /* Memory Barrier */

		while (flags[rankB] == 1 && data == rankB);
			__sync_synchronize(); /* Memory Barrier */

		/* Critical section */
		printf("Thread %ld entered critical section.\n", rankA);

		/* Clear flag to let the other thread enter its critical sectioin */
		flags[rankA] = 0;
		__sync_synchronize(); /* Memory Barrier */
	}

	return NULL;
}

int main()
{
	pthread_t th1, th2;

	pthread_create(&th1, NULL, thread_f, (void *)0);
	pthread_create(&th2, NULL, thread_f, (void *)1);

	pthread_join(th1, NULL);
	pthread_join(th2, NULL);

	return 0;
}
