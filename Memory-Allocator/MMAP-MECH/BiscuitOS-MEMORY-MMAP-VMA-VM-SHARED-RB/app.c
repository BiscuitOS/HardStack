// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: VMA SHARED.RB / SHARED.RB_SUBTREE_LAST
 *
 * (C) 2023.12.27 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define FILE_PATH	"/dev/BiscuitOS-VMA"
#define FILE_PATH2	"/mnt/BiscuitOS.txt"
#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)
#define BISCUITOS_IO	0xBD
#define CONSULT_RMAP	_IO(BISCUITOS_IO, 0x00)
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main()
{
	int pid, fd;
	char *base;

	fd = open(FILE_PATH2, O_RDWR);
	if (fd < 0)
		errExit("OPEN FAILED.\n");

	/* FILE-MAPPING VIRTUAL MEMORY */
	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    0);
	if (base == MAP_FAILED)
		errExit("MMAP FAILED\n");

	/* WRITE Ops, Trigger #PF MAPPING PAGECACHE */
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
