// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: SPEICAL VIRTUAL MEMORY
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

#define DEV_PATH	"/dev/BiscuitOS-SPECIAL"
#define BISCUITOS_IO	0xBD
#define BS_ALLOC	_IO(BISCUITOS_IO, 0x00)
#define MAP_VADDR	0x6000000000
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main()
{
	char *base;
	int fd;

	/* OPEN FILE */
	fd  = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* MAPPING SPECIAL MEMORY */
	if (ioctl(fd, BS_ALLOC, MAP_VADDR) < 0)
		errExit("IOCTL FAILED.\n");

	/* USE MEMORY */
	base = (char *)MAP_VADDR;
	/* Write OPS, Trigger #PF */
	*base = 'B';
	printf("MAP: %#lx => %c\n", (unsigned long)base, *base);

	close(fd);

	return 0;
}
