/*
 * NULL: 0 virtual address (Userspace)
 *
 * (C) 2021.05.01 <buddy.zhang@aliyun.com>
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
#include <sys/mman.h>

/* PATH for device */
#define DEV_PATH		"/dev/BiscuitOS"
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_NULL		_IO(BISCUITOS_IO, 0x00)

int main()
{
	int rvl = 0;
	int fd;

	mmap(NULL, 4096, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

	/* open device */
	fd = open(DEV_PATH, O_RDWR);

	/* ioctl */
	ioctl(fd, BISCUITOS_NULL, (unsigned long)0);

	/* only debug */
	printf("PID: %ld\n", (long)getpid());
	sleep(25);

	close(fd);
	return 0;
}
