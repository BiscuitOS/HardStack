/*
 * Paging Mechanism: PTE Present "P"
 *
 * (C) 2021.01.10 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PAGE_SIZE		4096
#define BISCUITOS_MAP_SIZE	(16 * PAGE_SIZE)
#define BISCUITOS_PATH		"/dev/BiscuitOS"

int main()
{
	unsigned long *val;
	char *default_base;
	int fd;

	/* open */
	fd = open(BISCUITOS_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", BISCUITOS_PATH);
		return -1;
	}

	/* mmap */
	default_base = (char *)mmap(NULL, BISCUITOS_MAP_SIZE,
					  PROT_READ | PROT_WRITE,
					  MAP_SHARED,
					  fd, 
					  0);
	if (!default_base) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -1;
	}

	val = (unsigned long *)default_base;
	/* Trigger page fault */
	*val = 88520;
	printf("=> %#lx\n", *val);

	/* Hold 3s */
	sleep(3);

	/* unmap */
	munmap(default_base, BISCUITOS_MAP_SIZE);
	close(fd);

	printf("Paging mechanism Applicatiin on BiscuitOS.\n");
	return 0;
}
