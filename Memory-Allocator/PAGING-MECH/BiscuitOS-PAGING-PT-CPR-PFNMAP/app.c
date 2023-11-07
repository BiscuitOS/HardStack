// SPDX-License-Identifier: GPL-2.0
/*
 * Copy-Page-Range: PFNMAP
 *
 * (C) 2023.11.05 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#define FILE_PATH	"/dev/BiscuitOS-CPR"
#define MAP_VADDR	0x6000000000

int main()
{
	char *addr;
	int fd;

	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", FILE_PATH);
		return -1;
	}

	addr = mmap((void *)MAP_VADDR, MAP_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE,
			fd, 
			0);
	if (!addr) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -1;
	}
	/* Father Write Ops, Trigger #PF */
	*addr = 'B';
	/* Father Read Ops, Don't Trigger #PF */
	printf("%#lx ==> %c\n", (unsigned long)addr, *addr);

	/* Copy-Page-Range */
	if (fork() == 0) {
		/* Son Write Ops, Trigger #Pf With COW */
		*addr = 'C';
		/* Son Read Ops, Don't Trigger #PF */
		printf("Son %#lx => %c\n", (unsigned long)addr, *addr);
	} else {
		sleep(1);
		/* Father Write Ops, Don't Trigger #PF */
		*addr = 'D';
		/* Father Read Ops, Don't Trigger #PF */
		printf("Father %#lx => %c\n", (unsigned long)addr, *addr);
	}

	munmap(addr, MAP_SIZE);
	close(fd);

	return 0;
}
