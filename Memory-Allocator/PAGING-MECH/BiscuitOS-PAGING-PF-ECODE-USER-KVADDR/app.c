// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault ERROR CODE: PF_USER Access KVADDR
 *
 * (C) 2023.09.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define DEV_PATH		"/dev/BiscuitOS"
#define BISCUITOS_IO		0xAE
#define BISCUITOS_FAULT		_IO(BISCUITOS_IO, 0x00)

int main()
{
	unsigned long base;
	int fd;

	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* Get Kernel Space Virtual Address */
	base = (unsigned long)ioctl(fd, BISCUITOS_FAULT, (unsigned long)0);
	if (base == -ENOMEM) {
		printf("ERROR: IOCTL BISCUITOS_FAULT.\n");
		goto out;
	}

	/* Write Ops, Trigger #PF with PF_USER */
	*(char *)base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("USER %#lx => %c\n", (unsigned long)base, *(char *)base);

out:
	close(fd);
	return 0;
}
