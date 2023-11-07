// SPDX-License-Identifier: GPL-2.0
/*
 * Copy-Page-Range(CPR): FILE-MAPPING
 *
 * (C) 2023.11.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)
#define FILE_PATH	"/BiscuitOS.txt"

int main()
{
	char *base;
	int fd;

	/* OPEN FILE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("%s Open Failed.\n", FILE_PATH);
		exit (-1);
	}

	/* FILE-MAPPING AND ALLOC MEMORY */
	base = mmap((void *)MAP_VADDR, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF */
	*base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("%#lx => %c\n", (unsigned long)base, *base);

	/* CPR? */
	if (fork() == 0) { /* Don't Copy-Page-Range and Empty PT */
		/* Son Write Ops, Trigger #PF */
		*base = 'C';
	} else {
		sleep(0.5);
		/* Father Write Ops, Don't Trigger #PF */
		*base = 'D';
	}

	sleep(-1); /* Just For Debug */
	munmap(base, PAGE_SIZE);
	close(fd);

	return 0;
}
