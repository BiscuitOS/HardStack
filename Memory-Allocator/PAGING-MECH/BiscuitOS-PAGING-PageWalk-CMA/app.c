// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk with CMA
 *
 * (C) 2023.08.01 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define DEV_PATH		"/dev/BiscuitOS-PageTable"
#define HPAGE_SIZE		(8 * 1024 * 1024)
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

	base = mmap(NULL, HPAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -1;
	}

	/* Write Ops, trigger PageFault */
	*(char *)base = 'B';
	/* Read Ops. Dont trigger PageFault */
	printf("%#lx => %c\n", (unsigned long)base, *(char *)base);

	sleep(1);
	ioctl(fd, BS_WALK_PT, (unsigned long)base);

	munmap(base, HPAGE_SIZE);
	close(fd);

	return 0;
}
