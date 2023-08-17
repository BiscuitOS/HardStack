// SPDX-License-Identifier: GPL-2.0
/*
 * PageMap with SWAP
 *
 * (C) 2023.08.13 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE		(4096)

static int detect_physcial_address(unsigned long vaddr,
			unsigned long *paddr, uint64_t *entry)
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

	/* Obtain physical page information */
	*entry = item;
	pfn = (((uint64_t)1 << 55) - 1) & item;
	*paddr = (pfn * pagesize) + pgoff;
	close(fd);

	return 0;
}

int main()
{
	unsigned long phys;
	uint64_t entry;
	void *base;

	/* alloc virtual memory */
	base = mmap(NULL, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS,
		    -1, 0);
	if (!base) {
		printf("Alloc Virtual memory failed.\n");
		return -ENOMEM;
	}

	/* alloc physical memory */
	*(char *)base = 'B';

	/* Reclaim and Swap out */
	madvise(base, PAGE_SIZE, MADV_PAGEOUT);
	sleep(1); /* SWAP OUT from SWAP CACHE */

	/* Detect physical information */
	if (!detect_physcial_address((unsigned long)base, &phys, &entry)) {
		printf("Vaddr: %#lx PHYS: %#lx Entry %#lx\n", 
				(unsigned long)base, phys, entry);
	}

	sleep(-1); /* Just for debug */

	/* free memory */
	munmap(base, PAGE_SIZE);

	return 0;
}
