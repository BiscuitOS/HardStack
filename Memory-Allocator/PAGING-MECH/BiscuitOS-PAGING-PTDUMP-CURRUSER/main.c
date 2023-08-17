// SPDX-License-Identifier: GPL-2.0
/*
 * PTDUMP: DUMP Current Userspace
 *
 * (C) 2023.08.12 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>

#define CURR_USER	"/sys/kernel/debug/page_tables/current_user"
#define PAGE_SIZE	4096

int main()
{
	void *buffer;
	int fd;

	fd = open(CURR_USER, O_RDWR);
	if (fd < 0) {
		printf("Open %s failed\n", CURR_USER);
		return -1;
	}

	buffer = malloc(PAGE_SIZE);
	if (!buffer) {
		printf("ERROR Memory.\n");
		close(fd);
		exit(-1);
	}
	printf("BUFFER %#lx\n", (unsigned long)buffer);

	/* Read PageTable */
	read(fd, buffer, PAGE_SIZE);

	/* DUMP Context */
	printf("%s", (char *)buffer);

	free(buffer);
	close(fd);

	return 0;
}
