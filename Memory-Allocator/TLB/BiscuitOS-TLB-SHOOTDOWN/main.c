// SPDX-License-Identifier: GPL-2.0
/*
 * FLUSH Shootdown
 *
 * (C) 2023.07.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#define PAGE_SIZE	4096
static unsigned long *shared_address;

void *thread_func1(void *arg)
{
	while (1) {
		sleep(0.5);
		madvise(shared_address, PAGE_SIZE, MADV_DONTNEED);
	}
}

void *thread_func2(void *arg)
{
	while (1) {
		sleep(1);
		printf("Read %#lx\n", *shared_address);
	}
}

int main()
{
	pthread_t th1, th2;
	void *base;

	/* mmap */
	base = mmap(NULL, 
		    PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	shared_address = (unsigned long *)base;
	/* Trigger page fault */
	*shared_address = 0x88520;

	pthread_create(&th1, NULL, thread_func1, NULL);
	pthread_create(&th2, NULL, thread_func2, NULL);

	pthread_join(th1, NULL);
	pthread_join(th2, NULL);

	sleep(-1);

	/* unmap */
	munmap(base, PAGE_SIZE);

	return 0;
}
