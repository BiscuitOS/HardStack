/*
 * Anonymous file on BiscuitOS (Userspace Part+)
 *
 * (C) 2020.10.24 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

/* PATH for devnode */
#define BISCUITOS_NODE		"/dev/BiscuitOS"
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_SET		_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_GET		_IO(BISCUITOS_IO, 0x01)

int main()
{
	int anonymous_fd;
	int fd;

	/* Open BiscuitOS node */
	fd = open(BISCUITOS_NODE, O_RDWR);
	if (fd < 0) {
		printf("ERROR[%d]: open %s failed.\n", fd, BISCUITOS_NODE);
		return fd;
	}

	/* Create Anonymous file by ioctl() */
	anonymous_fd = ioctl(fd, BISCUITOS_SET, (unsigned long)0);
	if (anonymous_fd < 0) {
		printf("ERROR[%d]: Anonymous file failed.\n", anonymous_fd);
		return anonymous_fd;
	}

	/* Anonymous fd */
	printf("Anonymous file-FD: %d\n", anonymous_fd);

	/* close */
	close(fd);

	return 0;
}
