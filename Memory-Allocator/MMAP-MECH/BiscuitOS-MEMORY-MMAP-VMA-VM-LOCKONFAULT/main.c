// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: VMA VM_LOCKONFAULT
 *
 * (C) 2023.12.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#define MAP_VADDR	(0x6000000000)
#define MAP_SIZE	(4096)
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main()
{
	void *mem;

	/* LAZYALLOC ANONYMOUS MEMORY */
	mem = mmap((void *)MAP_VADDR, MAP_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS,
		   -1,
		   0);
	if (mem == MAP_FAILED)
		errExit("MMAP FAILED\n");

	/* MCL_ONFAULT: MARK VM_LOCKONFAULT */
	if (mlockall(MCL_CURRENT | MCL_FUTURE | MCL_ONFAULT) != 0)
		errExit("MLOCK FAILED\n");

	/* ACCESS */
	*(char *)mem = 'B'; /* Write Ops Trigger #PF */
	printf("MMAP: %#lx => %c\n", (unsigned long)mem, *(char *)mem);

	/* UNLOCK */
	munlockall();

	/* RECLAIM */
	munmap(mem, MAP_SIZE);

	return 0;
}
