// SPDX-License-Identifier: GPL-2.0
/*
 * ONDEMAND: IOCTL
 *
 * (C) 2023.11.20 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define errExit(msg)		do { perror(msg); \
				     exit(EXIT_FAILURE); } while (0)
#define MAP_VADDR		(0x6000000000)
#define MAP_SIZE		0x1000
#define FILE_PATH		"/dev/BiscuitOS-ONDEMAND"

#define BISCUITOS_IO		0xBD
#define BISCUITOS_ONDEMAND	_IO(BISCUITOS_IO, 0x00)

int main()
{
	void *addr;
	int fd;

	/* OPEN SPECIAL FILE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0)
		errExit("ERROR: OPEN\n");

	/* ALLOC ANONYMOUS MEMORY */
	addr = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (addr == MAP_FAILED)
		errExit("ERROR: mmap\n");

	/* ONDEMAND */
	ioctl(fd, BISCUITOS_ONDEMAND, (unsigned long)addr);

	
	/* WRITE Ops, Don't Trigger #PF */
	*(char *)addr = 'H';
	printf("ONDEMAND-IOCTL: %#lx => %s\n",
				(unsigned long)addr, (char *)addr);

	/* RECLAIM */
	munmap(addr, MAP_SIZE);
	close(fd);

	return 0;
}
