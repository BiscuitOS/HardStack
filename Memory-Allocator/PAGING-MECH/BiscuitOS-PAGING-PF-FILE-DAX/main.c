// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with DAX
 *   CMDLINE: memmap=8M!128M  
 *
 * (C) 2023.09.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define HPAGE_SIZE	(4 * 2 * 1024 * 1024)
#define MAP_VADDR	(0x6000000000)
#define FILE_PATH	"/mnt/DAX/BiscuitOS.txt"

int main()
{
	void *base;
	int fd;

	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("Open %s failed.\n", FILE_PATH);
		exit (-1);
	}

	/* sys_mmap */
	base = mmap((void *)MAP_VADDR, HPAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED |
		    MAP_FIXED_NOREPLACE,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		close(fd);
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF */
	*(char *)base = 'B';

	/* Read Ops, Trigger #PF */
	printf("DAX %#lx => %c\n", (unsigned long)base, *(char *)base);

	munmap(base, HPAGE_SIZE);
	close(fd);

	return 0;
}
