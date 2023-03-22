// SPDX-License-Identifier: GPL-2.0
/*
 * Write Data
 *
 * (C) 2020.10.06 <buddy.zhang@aliyun.com>
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
static const char *astr = "Hello BiscuitOS";

int main()
{
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	if (write(fd, astr, strlen(astr)) != strlen(astr)) {
		printf("ERROR: BAD write!\n");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}
