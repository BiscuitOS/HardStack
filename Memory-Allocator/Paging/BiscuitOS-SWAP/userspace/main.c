/*
 * Swap Mechanism on BiscuitOS
 *
 * (C) 2021.03.16 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <sys/ioctl.h>

#define PAGE_SIZE		4096
#define BISCUITOS_MAP_SIZE	(16 * PAGE_SIZE)
#define BISCUITOS_PATH		"/dev/BiscuitOS"

/* IOCTL */
#define BISCUITOS_IO		0xBD
#define BISCUITOS_SWAP_OUT	_IO(BISCUITOS_IO, 0x00)

int main()
{
	unsigned long *val;
	unsigned long data;
	char *default_base;
	int fd, ret;

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
	printf("=> %ld\n", *val);

	/* SWAP OUT */
	ret = ioctl(fd, BISCUITOS_SWAP_OUT, (unsigned long)0);
	if (ret) {
		printf("ERROR: BISCUITOS_SWAP_OUT Failed.\n");
		goto err;
	}

	/* Trigger SWAP IN */
	data = *val;
	printf("=> %ld\n", data);

	/* unmap */
	munmap(default_base, BISCUITOS_MAP_SIZE);
	close(fd);

	printf("Paging mechanism Applicatiin on BiscuitOS.\n");
	return 0;

err:
	/* unmap */
	munmap(default_base, BISCUITOS_MAP_SIZE);
	close(fd);

	return ret;
}
