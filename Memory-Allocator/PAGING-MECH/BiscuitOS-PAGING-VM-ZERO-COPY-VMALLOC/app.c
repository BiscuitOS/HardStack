// SPDX-License-Identifier: GPL-2.0
/*
 * ZERO COPY: VMALLOC
 * 
 * (C) 2023.11.21 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define FILE_PATH	"/dev/BiscuitOS-ZERO"
#define MAP_VADDR	(0x6000000000)
#define MAP_SIZE	(4096)
/* IOCTL */
#define ZC_IO		0xBD
#define ZC_COPY		_IO(ZC_IO, 0x00)

int main()
{
	void *mem;
	int fd;

	/* OPEN FILE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0)
		exit(-1);

	/* PREALLOC VMALLOC MEMORY */
	mem = mmap((void *)MAP_VADDR, MAP_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_SHARED,
		   fd,
		   0);
	if (mem == MAP_FAILED)
		exit(-1);

	/* ACCESS */
	sprintf((char *)mem, "Hello BiscuitOS");

	/* ZERO-COPY TO KERNEL */
	ioctl(fd, ZC_COPY, 0);

	/* RECLAIM */
	munmap(mem, MAP_SIZE);
	close(fd);

	return 0;
}
