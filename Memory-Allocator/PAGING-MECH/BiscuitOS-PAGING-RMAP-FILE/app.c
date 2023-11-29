// SPDX-License-Identifier: GPL-2.0
/*
 * REVERSE MAPPING: FILE MAPPING
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
#define FILE_MPATH	"/mnt/BiscuitOS.txt"
#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)
#define BISCUITOS_IO	0xBD
#define CONSULT_RMAP	_IO(BISCUITOS_IO, 0x00)

int main()
{
	char *base;
	int pid, fd;

	/* OPEN FILE */
	fd = open(FILE_MPATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open\n");
		return -1;
	}

	/* ALLOC FILE-MAPPING MEMORY */
	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    0);
	if (base == MAP_FAILED) {
		close(fd);
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* WRITE Ops, Trigger #PF MAPPING ANON-PAGE */
	*base = 'H';

	/* FORK 1ST */
	pid = fork();

	/* SON */
	if (pid == 0) {
		/* READ ONLY, Don't COW  */
		printf("SON-%d: %s", getpid(), base);

		/* FORK 2ND */
		pid = fork();

		/* GRANDSON */
		if (pid == 0) {
			/* READ ONLY, Don't COW */
			printf("GSON-%d: %s", getpid(), base);
		} else {
			int rfd;

			sleep(1);
			/* SON CONSULT REVERSE MAPPING */
			rfd = open(FILE_PATH, O_RDWR);
			ioctl(rfd, CONSULT_RMAP, (unsigned long)base);
			close(rfd);
		}
	} else {
		sleep(1.5);
		/* FATHER READ ONLY, Don't COW */
		printf("FATHER-%d: %s", getpid(), base);
	}

	sleep(-1); /* JUST FOR DEBUG */

	/* RECLAIM */
	munmap(base, MAP_SIZE);
	close(fd);

	return 0;
}
