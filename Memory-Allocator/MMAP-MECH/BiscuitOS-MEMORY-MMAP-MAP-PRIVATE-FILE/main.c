// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: FILE-MAPPING MAP_PRIVATE
 *
 * (C) 2023.12.30 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_VADDR	(0x6000000000)
#define MAP_SIZE	(4096)
#define FILE_PATH	"/mnt/BiscuitOS.txt"
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main()
{
	int pid, fd;
	void *mem;
	char *ch;

	/* OPEN FILE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0)
		errExit("OPEN FAILED.\n");

	/* LAZYALLOC FILE-MAPPING MEMORY */
	mem = mmap((void *)MAP_VADDR, MAP_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_PRIVATE,
		   fd,
		   0);
	if (mem == MAP_FAILED)
		errExit("MMAP FAILED.\n");

	/* ACCESS, READ OPS Trigger #PF */
	ch = *(char *)mem;
	printf("FATHER: %#lx on %c\n", (unsigned long)mem, ch);

	/* FORK */
	pid = fork();

	if (pid == 0) {
		/* CHILD ACCESS: COW */
		*(char *)mem = 'D'; /* Write Ops, Trigger #PF */
		printf("SON: %c\n", *(char *)mem);
	} else {
		sleep(1);

		printf("FATHER-R: %c\n", *(char *)mem);
		/* FATHER ACCESS: PAGECACHE */
		*(char *)mem = 'E'; /* Write Ops, Trigger #PF */
		printf("FATHER-W: %c\n", *(char *)mem);
	}

	/* RECLAIM */
	munmap(mem, MAP_SIZE);

	return 0;
}
