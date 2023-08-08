// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk DeepMind PTE-LOCK
 *
 * (C) 2023.07.31 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define DEV_PATH		"/dev/BiscuitOS-PageTable"
#define PAGE_SIZE		(4 * 1024)
#define VIRTUAL_ADDRESS		(0x6000000000)
#define BISCUITOS_IO		0xBD
#define BS_WALK_PT		_IO(BISCUITOS_IO, 0x00)

static void *base;
static int fd;

void *p1_func(void *id)
{
	ioctl(fd, BS_WALK_PT, (unsigned long)base);	
}

void *p2_func(void *id)
{
	sleep(2);
	munmap(base, PAGE_SIZE);
	printf("P2 Release PageTable.\n");
}

int main()
{
	pthread_t p1, p2;

	/* open file */
	fd  = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	base = mmap((void *)VIRTUAL_ADDRESS, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS |
		    MAP_FIXED_NOREPLACE,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -1;
	}

	/* Read Ops. Trigger PageFault */
	printf("%#lx => %c\n", (unsigned long)base, *(char *)base);

	pthread_create(&p1, NULL, p1_func, NULL);
	pthread_create(&p2, NULL, p2_func, NULL);

	pthread_join(p2, NULL);
	pthread_join(p1, NULL);

	sleep(-1);
	munmap(base, PAGE_SIZE);
	close(fd);

	return 0;
}
