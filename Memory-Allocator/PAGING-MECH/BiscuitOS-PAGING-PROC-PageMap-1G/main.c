// SPDX-License-Identifier: GPL-2.0
/*
 * PageMap with 1Gig Page
 *
 * (C) 2023.08.15 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

#define HPAGE_SIZE			(1 * 1024 * 1024 * 1024)
#ifndef MAP_HUGE_1GB
#define HUGETLB_FLAG_ENCODE_SHIFT	26
#define MAP_HUGE_1GB			(30 << HUGETLB_FLAG_ENCODE_SHIFT)
#endif

static int detect_physcial_address(unsigned long vaddr, unsigned long *paddr)
{
	int pagesize = getpagesize();
	unsigned long vpage_index;
	unsigned long pfn, pgoff;
	unsigned long voffset;
	uint64_t item = 0;
	int fd;

	vpage_index = vaddr / pagesize;
	voffset = vpage_index * sizeof(uint64_t);
	pgoff = vaddr % pagesize;

	/* open pagemap */
	fd = open("/proc/self/pagemap", O_RDONLY);
	if (fd < 0) {
		printf("Open /proc/self/pagemap failed.\n");
		return -EINVAL;
	}

	if (lseek(fd, voffset, SEEK_SET) < 0) {
		printf("ERROR: lseek failed.\n");
		close(fd);
		return -EINVAL;
	}

	if (read(fd, &item, sizeof(uint64_t)) != sizeof(uint64_t)) {
		printf("ERROR: read item error.\n");
		close(fd);
		return -EINVAL;
	}

	if ((((uint64_t)1 << 63) & item) == 0) {
		printf("Physical page doesn't alloc.\n");
		close(fd);
		return -EINVAL;
	}

	/* Obtain physical page information */
	pfn = (((uint64_t)1 << 55) - 1) & item;
	*paddr = (pfn * pagesize) + pgoff;
	close(fd);
	return 0;
}

int main()
{
	unsigned long phys;
	char *base;

	/* alloc Hugetlbfs */
	base = mmap(NULL, HPAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS |
		    MAP_HUGETLB | MAP_HUGE_1GB,
		    -1, 0);
	if (base == MAP_FAILED) {
		printf("Alloc Hugetlb page failed.\n");
		return -1;
	}
	*(char *)base = 'A'; /* Trigger #PF */

	/* Detect physical information */
	if (!detect_physcial_address((unsigned long)base, &phys)) {
		printf("\nVaddr: %#lx\nPHYS:  %#lx\n", 
				(unsigned long)base, phys);
	}

	munmap(base, HPAGE_SIZE);

	return 0;
}
