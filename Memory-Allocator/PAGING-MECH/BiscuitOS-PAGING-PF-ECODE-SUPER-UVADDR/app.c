// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault ERROR CODE: PF_SUPER Access UVADDR
 *
 * (C) 2023.09.07 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define DEV_PATH		"/dev/BiscuitOS"
#define BISCUITOS_IO		0xAE
#define BISCUITOS_FAULT		_IO(BISCUITOS_IO, 0x00)
#define MAP_VADDR		(0x6000000000)
#define MAP_SIZE		(4096)

int main()
{
	void *base;
	int fd, r;

	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1, 0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed\n");
		close(fd);
		exit(-1);
	}

	r = ioctl(fd, BISCUITOS_FAULT, (unsigned long)base);
	if (fd < 0) {
		printf("ERROR: IOCTL BISCUITOS_FAULT.\n");
		goto out;
	}

	/* Read Ops, Don't Trigger #PF */
	printf("U/S %#lx => %c\n", (unsigned long)base, *(char *)base);

out:
	munmap(base, MAP_SIZE);
	close(fd);
	return r;
}
