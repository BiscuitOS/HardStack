// SPDX-License-Identifier: GPL-2.0
/*
 * CLEAR_REFS: File-Mapped HugeTLB Memory
 *
 * (C) 2023.08.30 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define HPAGE_SIZE	(4 * 2 * 1024 * 1024)
#define MAP_VADDR	(0x6000000000)
#define HUGETLB_PATH 	"/mnt/BiscuitOS-hugetlbfs/hugepage"

int main()
{
	char *base, ch;
	int fd;

	fd = open(HUGETLB_PATH, O_RDWR | O_CREAT);
	if (fd < 0) {
		printf("Open file failed.\n");
		exit(-1);
	}

	base = mmap((void *)MAP_VADDR, HPAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
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

	munmap(base, HPAGE_SIZE);
	close(fd);

	return 0;
}
