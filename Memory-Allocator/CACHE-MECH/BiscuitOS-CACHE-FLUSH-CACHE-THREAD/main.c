// SPDX-License-Identifier: GPL-2.0
/*
 * FLUSH CACHE: CLFLUSH on USERSPACE
 *  - Must 4 CPUs
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <x86intrin.h>
#include <pthread.h>
#include <sched.h>

#define LOOP_COUNT	(1024 * 1024 * 16)

static long shared_memory;

void *pread_func(void *id)
{
	cpu_set_t cpuset;
	unsigned long i, data;

	CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);
	CPU_SET(1, &cpuset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (CPU_ISSET(0, &cpuset) || CPU_ISSET(1, &cpuset))
		printf("Read Thread Running on CPU0 or CPU1\n");

	for (i = 0; i < LOOP_COUNT; i++)
		data = shared_memory; /* Exclusive or Shared */

	pthread_exit(NULL);
}

void *pwrite_func(void *id)
{
	cpu_set_t cpuset;
	unsigned long i;

	CPU_ZERO(&cpuset);
	CPU_SET(2, &cpuset);
	CPU_SET(3, &cpuset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (CPU_ISSET(2, &cpuset) || CPU_ISSET(3, &cpuset))
		printf("Write Thread Running on CPU2 or CPU3\n");

	for (i = 0; i < LOOP_COUNT; i++) {
		shared_memory = i; /* Modify */
		_mm_clflush(&shared_memory); /* WriteBack */
	}

	pthread_exit(NULL);
}

int main()
{
	pthread_t pread, pwrite;
	struct timeval tv, ntv;

	gettimeofday(&tv, NULL);
	pthread_create(&pread, NULL, pread_func, NULL);
	pthread_create(&pwrite, NULL, pwrite_func, NULL);

	/* Wait thread finish */
	pthread_join(pread, NULL);
	pthread_join(pwrite, NULL);
	gettimeofday(&ntv, NULL);

	/* COST Time */
	printf("Cost Time: %ld nsec\n", ntv.tv_sec * 1000000 +
		ntv.tv_usec - tv.tv_sec * 1000000 - tv.tv_usec);

	return 0;
}
