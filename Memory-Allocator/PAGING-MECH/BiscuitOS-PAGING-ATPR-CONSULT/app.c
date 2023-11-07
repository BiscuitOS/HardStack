// SPDX-License-Identifier: GPL-2.0
/*
 * ATPR(Apply to page range): Consult PageTable
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
#include <sys/ioctl.h>

#define FILE_PATH		"/dev/BiscuitOS-ATPR"
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_CONSULT	_IO(BISCUITOS_IO, 0x00)

int main()
{
	void *addr;
	int fd;

	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", FILE_PATH);
		return -1;
	}

	/* Alloc Memory */
	addr = malloc(64);

	*(char *)addr = 'B';
	printf("%#lx => %c\n", (unsigned long)addr, *(char *)addr);
	sleep(0.2); /* Just for Debug */

	/* Consult Pagetable */
	ioctl(fd, BISCUITOS_CONSULT, (unsigned long)addr);

	free(addr);
	close(fd);

	return 0;
}
