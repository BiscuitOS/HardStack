// SPDX-License-Identifier: GPL-2.0
/*
 * PAGING MISC
 *
 * (C) 2023.11.23 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define DEV_PATH		"/dev/BiscuitOS"

int main()
{
	int fd;

	/* OPEN FILE */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* RECLAIM */
	close(fd);

	return 0;
}
