// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk with MMIO
 *
 * (C) 2023.07.31 <buddy.zhang@aliyun.com>
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
#define MMIO_SIZE		(4096)
#define MMIO_BASE		0xF0000000
#define BISCUITOS_IO		0xBD
#define BS_WALK_PT		_IO(BISCUITOS_IO, 0x00)

int main()
{
	void *base;
	int fd, fd_mmio;

	/* open file */
	fd  = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	fd_mmio = open("/dev/mem", O_RDWR);
	if (fd_mmio < 0) {
		printf("ERROR: Can't open /dev/mem\n");
		close(fd);
		return -1;
	}

	base = mmap(NULL, MMIO_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd_mmio,
		    MMIO_BASE);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd_mmio);
		close(fd);
		return -1;
	}

	/* Write Ops. Trigger PageFault */
	*(char *)base = 'B';
	/* Read Ops. Trigger PageFault */
	printf("%#lx => %c\n", (unsigned long)base, *(char *)base);

	sleep(1);
	ioctl(fd, BS_WALK_PT, (unsigned long)base);

	munmap(base, MMIO_SIZE);
	close(fd_mmio);
	close(fd);

	return 0;
}
