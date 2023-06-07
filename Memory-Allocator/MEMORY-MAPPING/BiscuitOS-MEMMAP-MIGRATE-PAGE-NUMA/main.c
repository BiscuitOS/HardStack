// SPDX-License-Identifier: GPL-2.0
/*
 * MEMMAP: MOVE PAGE ON NUMA
 *
 * (C) 2022.05.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <asm/unistd_64.h>

#define PAGE_SIZE		(4096)

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
	int node;

	base = (char *)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
			   MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
	if (base == MAP_FAILED) {
		printf("Alloc Virtual memory failed.\n");
		return -ENOMEM;
	}

	/* Detect physical information */
	if (!detect_physcial_address((unsigned long)base, &phys)) {
		printf("\nBefore Virtual-addr: %#lx\nPHYS-Address: %#lx\n", 
				(unsigned long)base, phys);
	}

	/* INFO: NUMA NODE */
	syscall(__NR_move_pages, 0, 1, &base, NULL, &node, 0);
	node = node == 0 ? 1 : 0; /* MOVE TO ANOTHER NUMA NODE */

	/* MOVE PAGE */
	syscall(__NR_move_pages, 0, 1, &base, &node, &node, 0);
	/* Detect physical information */
	if (!detect_physcial_address((unsigned long)base, &phys)) {
		printf("\nAfter Virtual-addr: %#lx\nPHYS-Address: %#lx\n", 
				(unsigned long)base, phys);
	}

	munmap(base, PAGE_SIZE);
	return 0;
}
