/*
 * Paging Mechanism: Mapping 4MiB Page With 32-Bit Paging
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

/* 4MiB Virtual Space */
#define BISCUITOS_MAP_SIZE	(1 << 22)
#define BISCUITOS_PATH		"/dev/BiscuitOS"

int main()
{
	unsigned long *val;
	char *default_base;
	char *range_base;
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
	printf("=> %#lx: %ld\n", (unsigned long)val, *val);

	/* Access offset 1M on 4M Page */
	range_base = (char *)((unsigned long)default_base + (1 << 20));
	val = (unsigned long *)range_base;
	/* Don't Trigger page fault */
	*val = 52088;
	printf("=> %#lx: %ld\n", (unsigned long)val, *val);

	/* Hold 3s */
	sleep(3);

	/* unmap */
	munmap(default_base, BISCUITOS_MAP_SIZE);
	close(fd);

	printf("Paging mechanism Applicatiin on BiscuitOS.\n");
	return 0;
}
