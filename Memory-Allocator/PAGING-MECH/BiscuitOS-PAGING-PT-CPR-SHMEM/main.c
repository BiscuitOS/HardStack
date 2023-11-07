// SPDX-License-Identifier: GPL-2.0
/*
 * Copy-Page-Range(CPR): SHARED MEMORY
 *
 * (C) 2023.11.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base;

	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF */
	*base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("SHARED-CPR %#lx => %c\n", (unsigned long)base, *base);


	/* NON-CPR and NON-COW */
	if (fork() == 0) { /* Don't Copy-Page-Range and EMPTY PT */
		/* Write Ops, Empty PT Trigger #PF */
		*base = 'C';
	} else {
		sleep(0.5);
		/* Write Ops, Don't Trigger #PF */
		*base = 'D';
	}

	sleep(-1); /* Just for debug */
	munmap(base, MAP_SIZE);

	return 0;
}
