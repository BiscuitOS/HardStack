// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with PFNMAP(RSVDMEM) Write-Protection
 *
 * (C) 2023.09.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAP_SIZE	(4096)
#define FILE_PATH	"/dev/BiscuitOS-PFNMAP"

int main()
{
	char *addr, ch;
	int fd;

	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", FILE_PATH);
		return -1;
	}

	addr = mmap(NULL, MAP_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd, 
			0);
	if (!addr) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -1;
	}

	/* Read Ops, Trigger #PF */
	ch = *addr;
	/* Read Ops, Don't Trigger #PF */
	printf("%#lx => %c\n", (unsigned long)addr, *addr);

	sleep(1); /* Just for Debug */
	/* Write Ops, Trigger #PF with Write-Protection */
	*addr = 'B';

	munmap(addr, MAP_SIZE);
	close(fd);

	return 0;
}
