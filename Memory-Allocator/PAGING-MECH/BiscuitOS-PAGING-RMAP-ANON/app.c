// SPDX-License-Identifier: GPL-2.0
/*
 * REVERSE MAPPING: ANONYMOUS MAPPING
 *
 * (C) 2023.11.22 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define FILE_PATH	"/dev/BiscuitOS-RMAP"
#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)
#define BISCUITOS_IO	0xBD
#define CONSULT_RMAP	_IO(BISCUITOS_IO, 0x00)

int main()
{
	char *base;
	int pid;

	/* ALLOC ANONYMOUS MEMORY */
	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* WRITE Ops, Trigger #PF MAPPING ANON-PAGE */
	*base = 'B';

	/* FORK 1ST */
	pid = fork();

	/* SON */
	if (pid == 0) {
		/* READ ONLY, Don't COW  */
		printf("SON-%d: %c\n", getpid(), *base);

		/* FORK 2ND */
		pid = fork();

		/* GRANDSON */
		if (pid == 0) {
			/* READ ONLY, Don't COW */
			printf("GSON-%d: %c\n", getpid(), *base);
		} else {
			int fd;

			sleep(1);
			/* SON CONSULT REVERSE MAPPING */
			fd = open(FILE_PATH, O_RDWR);
			ioctl(fd, CONSULT_RMAP, (unsigned long)base);
			close(fd);
		}
	} else {
		sleep(1.5);
		/* FATHER READ ONLY, Don't COW */
		printf("FATHER-%d: %c\n", getpid(), *base);
	}

	sleep(-1); /* JUST FOR DEBUG */

	/* RECLAIM */
	munmap(base, MAP_SIZE);

	return 0;
}
