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
#include <stdatomic.h>

int flags[2];
int data;

void *thread_f(void *rank)
{
	long rankA = (long)rank;
	long rankB = 1 - rankA;

	while (1) {
		atomic_store_explicit(&flags[rankA], 1, memory_order_release);
		atomic_store_explicit(&data, rankB, memory_order_release);

		do {
			rankB = atomic_load_explicit(&flags[rankA], memory_order_acquire);
		} while (rankB == 1 && atomic_load_explicit(&data, memory_order_acquire) == rankB);

		/* Critical section */
		printf("Thread %ld entered critical section.\n", rankA);

		/* Clear flag to let the other thread enter its critical sectioin */
		atomic_store_explicit(&flags[rankA], 0, memory_order_release);
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
