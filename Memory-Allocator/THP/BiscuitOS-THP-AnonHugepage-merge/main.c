/*
 * THP: Merge page to AnonHugepages
 *
 * (C) 2022.05.08 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <sys/mman.h>

#define HPAGE_SIZE		(2 * 1024 * 1024)
#define VIRTUAL_ADDRESS0	(0x6000000000)
#define VIRTUAL_ADDRESS1	(0x6000080000)
#define VIRTUAL_ADDRESS2	(0x6000100000)
#define VIRTUAL_ADDRESS3	(0x6000140000)
#define VIRTUAL_ADDRESS4	(0x6000180000)
#define MAP_FIXED_NOREPLACE	0x100000

/* alloc memory */
static char *
BiscuitOS_alloc(unsigned long vaddr, unsigned long size, int populate)
{
	int flags = populate ? MAP_POPULATE : 0;

	return mmap((void *)vaddr, size, 
			    PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS |
			    MAP_FIXED_NOREPLACE | flags, -1, 0);
}


int main()
{
	/* Region: 25% HP 25% page */
	BiscuitOS_alloc(VIRTUAL_ADDRESS0, HPAGE_SIZE / 4, 1);
	/* Region: 50% HP 25% page */
	BiscuitOS_alloc(VIRTUAL_ADDRESS1, HPAGE_SIZE / 4, 0);
	/* Region: 62.5 HP 37.5% page */
	BiscuitOS_alloc(VIRTUAL_ADDRESS2, HPAGE_SIZE / 8, 1);
	/* Region: 75% HP 50% page */
	BiscuitOS_alloc(VIRTUAL_ADDRESS3, HPAGE_SIZE / 8, 1);
	/* Region: 100% HP 50% page. 
	 * Trigger THP: No less then 2MiB HP and 50% page.
	 */
	BiscuitOS_alloc(VIRTUAL_ADDRESS4, HPAGE_SIZE / 4, 0);

	/* Just for debug */
	sleep(-1);

	/* unmap */
	munmap((void *)VIRTUAL_ADDRESS0, HPAGE_SIZE);

	return 0;
}
