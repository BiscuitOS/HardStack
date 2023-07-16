// SPDX-License-Identifier: GPL-2.0
/*
 * FLUSH TLB on Userspace
 *
 * (C) 2023.07.15 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define BISCUITOS_MAP_SIZE	(4096)
#define BISCUITOS_PATH		"/dev/BiscuitOS"
#define BISCUITOS_IO		0xBD
#define BS_FLUSH_TLB		_IO(BISCUITOS_IO, 0x00)

int main()
{
	void *addr;
	int fd;

	/* open */
	fd = open(BISCUITOS_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", BISCUITOS_PATH);
		return -1;
	}

	/* mmap */
	addr = mmap(NULL, BISCUITOS_MAP_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd, 
			0);
	if (!addr) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -1;
	}

	/* use */
	*(char *)addr = 'B';
	printf("%#lx => %c\n", (unsigned long)addr, *(char *)addr);

	/* FLUSH TLB */
	ioctl(fd, BS_FLUSH_TLB, (unsigned long)addr);

	/* unmap */
	munmap(addr, BISCUITOS_MAP_SIZE);
	close(fd);

	return 0;
}
