/*
 * BiscuitOS
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

/* PATH for device */
#define DEV_PATH	"/dev/Demo"

int main()
{
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("Can't open %s\n", DEV_PATH);
		return -1;
	}

	write(fd, "BiscuitOS", strlen("BiscuitOS"));
out:
	close(fd);
	return 0;
}
