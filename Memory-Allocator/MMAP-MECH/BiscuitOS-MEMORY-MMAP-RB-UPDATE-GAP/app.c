// SPDX-License-Identifier: GPL-2.0
/*
 * MEMORY MMAP: UPDATE GAP
 *
 * (C) 2023.12.17 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define DEV_PATH	"/dev/BiscuitOS"
#define BISCUITOS_IO	0xBD
#define MMAP_GAP	_IO(BISCUITOS_IO, 0x00)
#define MAP_VADDR1	0x6000000000
#define MAP_VADDR2	0x6000100000
#define MAP_VADDR3	0x6000200000
#define MAP_SIZE	(4 * 1024)
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

/* ALLOC ANONYMOUS MEMORY */
static void *alloc_anonymous_memory(unsigned long vaddr, unsigned long size)
{
	return mmap((void *)vaddr, size, PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

int main()
{
	void *mem1, *mem2, *mem3;
	int fd;

	/* OPEN DEVICE */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0)
		errExit("OPEN FAILED.\n");

	mem1 = alloc_anonymous_memory(MAP_VADDR1, MAP_SIZE);
	mem2 = alloc_anonymous_memory(MAP_VADDR2, MAP_SIZE);
	mem3 = alloc_anonymous_memory(MAP_VADDR3, MAP_SIZE);

	/* GAP OPS */
	if (ioctl(fd, MMAP_GAP, (unsigned long)mem2) < 0)
		errExit("IOCTL FAILED.\n");

	sleep(-1); /* JUST FOR DEBUG */
	/* RECLAIM */
	munmap(mem1, MAP_SIZE);
	munmap(mem2, MAP_SIZE);
	munmap(mem3, MAP_SIZE);
	close(fd);
	return 0;
}
