// SPDX-License-Identifier: GPL-2.0
/*
 * PTDUMP: DUMP Current Kernel Space
 *
 * (C) 2023.08.12 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define CURR_USER	"/sys/kernel/debug/page_tables/current_kernel"
#define PAGE_SIZE	(8 * 4096)

int main()
{
	void *buffer;
	int fd;

	fd = open(CURR_USER, O_RDWR);
	if (fd < 0) {
		printf("Open %s failed\n", CURR_USER);
		return -1;
	}

	buffer = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
		      MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
	if (buffer == MAP_FAILED) {
		printf("ERROR Memory.\n");
		close(fd);
		exit(-1);
	}
	printf("BUFFER %#lx\n", (unsigned long)buffer);

	/* Read PageTable */
	read(fd, buffer, PAGE_SIZE);

	/* DUMP Context */
	printf("%s", (char *)buffer);

	munmap(buffer, PAGE_SIZE);
	close(fd);

	return 0;
}
