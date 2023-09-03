// SPDX-License-Identifier: GPL-2.0-only
/*
 * PageFault with File HugeTLB
 *
 * (C) 2023.09.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	4096
#define HUGETLB_PATH	"/mnt/BiscuitOS-hugetlbfs/hugepage"

int main()
{
	char *base;
	int fd;

	/* Open Hugepage */
	fd = open(HUGETLB_PATH, O_RDWR | O_CREAT);
	if (fd < 0) {
		printf("ERROR: failed open %s\n", HUGETLB_PATH);
		return -EINVAL;
	}

	/* mmap */
	base = (char *)mmap(NULL, 
			    MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED,
			    fd,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	/* Write Ops, Trigger #PF */
	*base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("%#lx => %c\n", (unsigned long)base, *base);

	/* unmap */
	munmap(base, MAP_SIZE);
	close(fd);

	return 0;
}
