// SPDX-License-Identifier: GPL-2.0
/*
 * CLEAR_REFS: File-Mapped Memory
 *
 * (C) 2023.08.30 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base, ch;
	int fd;

	fd = open("/tmp/BiscuitOS.txt", O_RDWR);
	if (fd < 0) {
		printf("Open file failed.\n");
		exit(-1);
	}

	base = mmap((void *)MAP_VADDR, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	while (1) {
		sleep(random() % 5);
		/* Access or Reference Memory */
		ch = *base; /* Read Ops */
	}

	munmap(base, PAGE_SIZE);
	close(fd);

	return 0;
}
