/*
 * THP: ShmemHugePage on File private
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

#define BISCUITOS_FILE_PATH	"/BiscuitOS-tmpfs/BiscuitOS"
#define HPAGE_SIZE		(2 * 1024 * 1024)
#define BISCUITOS_MAP_SIZE	(4 * HPAGE_SIZE)
#define VIRTUAL_ADDRESS		(0x6000000000)
#define MAP_FIXED_NOREPLACE	0x100000

int main()
{
	char *base;
	int fd;

	/* open file on huge tmpfs */
	fd = open(BISCUITOS_FILE_PATH, O_RDWR | O_CREAT);
	if (fd < 0) {
		printf("ERROR: Open %s failed.\n", BISCUITOS_FILE_PATH);
		return -EBUSY;
	}

	/* Expand file on disk */
	write(fd, "BiscuitOS ShmemHugepage", 23);

	/* mmap */
	base = (char *)mmap((void *)VIRTUAL_ADDRESS, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS |
			    MAP_FIXED_NOREPLACE,
			    fd,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	printf("BiscuitOS Range: %#lx - %#lx\n", (unsigned long)base,
				(unsigned long)base + BISCUITOS_MAP_SIZE);
	/* access hugepage */
	base[HPAGE_SIZE * 0] = 'A'; /* Trigger 1st SHMHUGE */

	/* Just for debug */
	sleep(-1);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);
	close(fd);

	return 0;
}
