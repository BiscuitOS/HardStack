// SPDX-License-Identifier: GPL-2.0
/*
 * FLUSH CACHE on Streaming DMA
 *
 * (C) 2023.04.03 <buddy.zhang@aliyun.com>
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
#define DEV_PATH		"/dev/Broiler-DMA"
#define BUF_LEN			64
static const char str[] = "Hello BiscuitOS!";

int main()
{
	char buffer[BUF_LEN];
	int fd, r;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* Write to Device */
	r = write(fd, str, strlen(str));
	if (r != strlen(str)) {
		printf("ERROR: BAD write!\n");
		r = -1;
		goto out;
	}

	/* Read from Device */
	r = read(fd, buffer, BUF_LEN);
	if (r < 0) {
		printf("ERROR: BAD read!\n");
		r = -1;
		goto out;
	}
	printf("DMA Context: %s\n", buffer);
	r = 0; /* Normal exit */
	
out:
	close(fd);
	return r;
}
