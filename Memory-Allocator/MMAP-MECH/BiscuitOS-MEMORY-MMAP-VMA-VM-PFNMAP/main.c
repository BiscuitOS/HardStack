// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: VMA VM_PFNMAP
 *
 *   CMDLINE ADD 'memmap=4K$0x10000000'
 *
 * (C) 2023.11.03 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAP_SIZE	(4 * 1024)
#define FILE_PATH	"/dev/mem"
#define RSVDMEM_BASE	0x10000000
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main()
{
	void *addr;
	int fd;

	/* OPEN FILE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0)
		errExit("OPEN FAILED\n");

	/* MAPPING: MARK VM_PFNMAP */
	addr = mmap(NULL, MAP_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd, 
			RSVDMEM_BASE);
	if (addr == MAP_FAILED)
		errExit("MMAP FAILED\n");

	/* ACCESS */
	*(char *)addr = 'B'; /* Write OPS Don't Trigger #PF */
	printf("MMAP %#lx => %c\n", (unsigned long)addr, *(char *)addr);

	/* RECLAIM */
	munmap(addr, MAP_SIZE);
	close(fd);

	return 0;
}
