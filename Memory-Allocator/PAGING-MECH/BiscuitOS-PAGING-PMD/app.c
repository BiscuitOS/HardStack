// SPDX-License-Identifier: GPL-2.0
/*
 * PMD(Page Middle Directory) APP
 *
 * (C) 2023.07.25 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/ioctl.h>

#define DEV_PATH		"/dev/BiscuitOS-PageTable"
#define PAGE_SIZE		4096
#define BISCUITOS_IO		0xBD
#define BS_WALK_PT		_IO(BISCUITOS_IO, 0x00)

int main()
{
	void *base;
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	base = malloc(PAGE_SIZE);
	if (!base) {
		printf("ERROR: No free Memory\n");
		close(fd);
		return -1;
	}

	/* Write Ops, Dont trigger PageFault */
	*(char *)base = 'B';
	/* Read Ops. Dont trigger PageFault */
	printf("%#lx => %c\n", (unsigned long)base, *(char *)base);

	sleep(1);
	ioctl(fd, BS_WALK_PT, (unsigned long)base);

	free(base);
	close(fd);

	return 0;
}
