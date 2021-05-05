/*
 * BiscuitOS MISC DD on Userspace
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
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

/* PATH for device */
#define DEV_PATH		"/dev/BiscuitOS"
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_NULL		_IO(BISCUITOS_IO, 0x00)

int main()
{
	int rvl = 0;
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* ioctl */
	rvl = ioctl(fd, BISCUITOS_NULL, (unsigned long)0);
	if (fd < 0) {
		printf("ERROR: IOCTL BISCUITOS_NULL.\n");
		goto out;
	}

	/* Normal ending */
	rvl = 0;
out:
	close(fd);
	return rvl;
}
