// SPDX-License-Identifier: GPL-2.0
/*
 * Huge-PageFault - COW(Copy-on-Write) Anonymous THP
 *
 * (C) 2023.10.15 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(2 * 1024 * 1024)
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base, ch;

	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	*base = 'B';

	if (fork() == 0) {
		/* Sun Write Ops, Trigger #PF COW */
		*base = 'D';
	} else {
		sleep(1);
		/* Father Write Ops, Trigger #PF COW */
		*base = 'C';
	}

	/* Read Ops, Don't Trigger #PF */
	printf("COW ANON-THP %#lx => %c\n", (unsigned long)base, *base);

	munmap(base, MAP_SIZE);

	return 0;
}
