/*
 * Misc Device
 *
 * (C) 2019.09.24 <buddy.zhang@aliyun.com>
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

/* Exchange data between kernel and userland */
struct Char_demo_pdata {
	int num;
};

/* PATH for device */
#define DEV_PATH	"/dev/Char_demo"

int main()
{
	struct Char_demo_pdata pdata;
	int rvl = 0;
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* Read information from Kernel */
	rvl = read(fd, &pdata, sizeof(pdata));
	if (rvl != sizeof(pdata)) {
		printf("BAD reading.\n");
		rvl = -1;
		goto out;
	}
	printf("Kernel information: %d\n", pdata.num);

	/* Write informtion to kernel */
	pdata.num = 0x28;
	rvl = write(fd, &pdata, sizeof(pdata));
	if (rvl != sizeof(pdata)) {
		printf("BAD write!\n");
		rvl = -1;
		goto out;
	}
	
	/* Normal ending */
	rvl = 0;
out:
	close(fd);
	return rvl;
}
