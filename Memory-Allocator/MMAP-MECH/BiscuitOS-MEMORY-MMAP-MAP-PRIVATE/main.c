// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: ANONYMOUS MAP_PRIVATE
 *
 * (C) 2023.12.17 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#define MAP_VADDR	(0x6000000000)
#define MAP_SIZE	(4096)

int main()
{
	void *mem;
	int pid;

	/* LAZYALLOC ANONYMOUS MEMORY */
	mem = mmap((void *)MAP_VADDR, MAP_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS,
		   -1,
		   0);
	if (mem == MAP_FAILED)
		exit(-1);

	/* ACCESS */
	*(char *)mem = 'B'; /* Write Ops Trigger #PF */
	printf("MMAP: %#lx => %c\n", (unsigned long)mem, *(char *)mem);

	/* FORK */
	pid = fork();

	if (pid == 0) {
		/* CHILD ACCESS: COW */
		*(char *)mem = 'D'; /* Write Ops, Trigger #PF */
		printf("SON: %c\n", *(char *)mem);
	} else {
		sleep(1);

		printf("FATHER-R: %c\n", *(char *)mem);
		/* FATHER ACCESS: REUSE */
		*(char *)mem = 'E'; /* Write Ops, Trigger #PF */
		printf("FATHER-W: %c\n", *(char *)mem);
	}

	/* RECLAIM */
	munmap(mem, MAP_SIZE);

	return 0;
}
