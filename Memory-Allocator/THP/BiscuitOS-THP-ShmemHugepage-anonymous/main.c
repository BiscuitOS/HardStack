/*
 * THP: ShmemHugePage on anonymous
 *
 * (C) 2022.05.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#define HPAGE_SIZE		(2 * 1024 * 1024)
#define BISCUITOS_MAP_SIZE	(4 * HPAGE_SIZE)
#define VIRTUAL_ADDRESS		(0x6000000000)
#define MAP_FIXED_NOREPLACE	0x100000

int main()
{
	char *base;

	/* mmap */
	base = (char *)mmap((void *)VIRTUAL_ADDRESS, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED | MAP_ANONYMOUS |
			    MAP_FIXED_NOREPLACE,
			    -1,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	printf("BiscuitOS Range: %#lx - %#lx\n", (unsigned long)base,
				(unsigned long)base + BISCUITOS_MAP_SIZE);
	/* access hugepage */
	base[HPAGE_SIZE * 0] = 'A'; /* Trigger 1st SHMHUGE */
	base[HPAGE_SIZE * 1] = 'B'; /* Trigger 2nd SHMHUGE */
	base[HPAGE_SIZE * 2] = 'C'; /* Trigger 3rd SHMHUGE */
	base[HPAGE_SIZE * 3] = 'D'; /* Trigger 4th SHMHUGE */

	/* Just for debug */
	sleep(-1);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
