// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Anonymous on WP REUSE
 *
 * (C) 2023.09.18 BuddyZhang1 <buddy.zhang@aliyun.com>
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
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF, Alloc Normal Page */
	*base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("ANON-WP-REUSE %#lx => %c\n", (unsigned long)base, *base);

	if (fork() == 0) {
		/* Write Ops, Trigger #PF and MAPCOUNT==2 COPY PAGE */
		*base = 'C';
	} else {
		sleep(0.5);
		/* Write Ops, Trigger #PF and MAPCOUNT==1 REUSE PAGE */
		*base = 'D';
	}

	sleep(-1); /* Just for Debug */

	munmap(base, MAP_SIZE);

	return 0;
}
