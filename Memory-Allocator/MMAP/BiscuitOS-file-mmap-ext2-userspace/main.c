/*
 * File mmap on ext2 filesystem (Userspace+)
 *
 * (C) 2021.04.02 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#define BISCUITOS_MAP_SIZE	4096

int main()
{
	unsigned long *val;
	char *base;
	int fd;

	/* Open */
	fd = open(BISCUITOS_FILE_PATH, O_RDWR | O_CREAT);
	if (fd < 0) {
		printf("ERROR: Open %s failed.\n", BISCUITOS_FILE_PATH);
		return -EBUSY;
	}

	/* expand file by write */
	write(fd, "BiscuitOS", strlen("BiscuitOS"));

	/* mmap */
	base = (char *)mmap(NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED,
			    fd,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	val = (unsigned long *)base;
	/* Trigger page fault */
	*val = 88520;
	printf("%#lx => %#lx\n", (unsigned long)val, *val);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);
	close(fd);

	return 0;
}
