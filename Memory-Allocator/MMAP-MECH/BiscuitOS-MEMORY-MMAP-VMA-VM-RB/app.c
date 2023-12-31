// SPDX-License-Identifier: GPL-2.0
/*
 * VMA: VMA VM_RB/RB_SUBTREE_GAP
 *
 * (C) 2023.12.26 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define DEV_PATH		"/dev/BiscuitOS-VMA"
#define BISCUITOS_IO		0xBD
#define TRAVER_VMA		_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_GET		_IO(BISCUITOS_IO, 0x01)

int main()
{
	int fd;

	/* OPEN FILE */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* TRAVER VMA */
	if (ioctl(fd, TRAVER_VMA, (unsigned long)0) < 0) {
		printf("ERROR: IOCTL TRAVER_VMA.\n");
		close(fd);
		return -1;
	}

	/* RECLAIM */
	close(fd);

	return 0;
}
