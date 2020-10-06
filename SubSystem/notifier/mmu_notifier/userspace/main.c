/*
 * BiscuitOS MMU notifier on Userspace
 *
 * (C) 2020.10.06 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

/* PATH for device */
#define DEV_PATH		"/dev/BiscuitOS"

int main()
{
	char *memory;
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* mmap */
	memory = mmap(NULL, 0x100000, PROT_READ | PROT_WRITE,
						MAP_SHARED, fd, 0);
	if (!memory) {
		printf("ERROR: mmap faied.\n");
		goto out;
	}

	/* un-mmap */
	munmap(memory, 0x100000);

	/* Normal ending */
	close(fd);
	return 0;
out:
	close(fd);
	return -1;
}
