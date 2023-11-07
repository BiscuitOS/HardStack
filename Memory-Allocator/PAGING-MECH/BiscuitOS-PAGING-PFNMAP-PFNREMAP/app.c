// SPDX-License-Identifier: GPL-2.0
/*
 * PFNMAP: PFNREMAP
 *
 * (C) 2023.11.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BISCUITOS_MAP_SIZE	(4096)
#define BISCUITOS_PATH		"/dev/BiscuitOS-PFNMAP"

int main()
{
	void *addr;
	int fd;

	/* OPEN DEVICE */
	fd = open(BISCUITOS_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", BISCUITOS_PATH);
		return -1;
	}

	/* MAPPING */
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

	/* ACCESS MEMORY */
	*(char *)addr = 'B';
	printf("PFNMAP %#lx => %c\n", (unsigned long)addr, *(char *)addr);

	sleep(-1); /* Just for DEBUG */

	/* RECLAIM */
	munmap(addr, BISCUITOS_MAP_SIZE);
	close(fd);

	return 0;
}
