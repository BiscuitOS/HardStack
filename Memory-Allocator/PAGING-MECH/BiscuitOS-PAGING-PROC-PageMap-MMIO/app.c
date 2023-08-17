// SPDX-License-Identifier: GPL-2.0
/*
 * PageMap with MMIO
 *
 * (C) 2023.08.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

/* Broiler MMIO Base Device */
#define MMIO_BASE		0xF0000000
#define MMIO_SIZE		4096

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

	printf("Pagemap Entry %#lx\n", item);
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
	unsigned long *val, phys;
	char *base;
	int fd;

	/* Open /dev/mem */
	fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		printf("ERROR: Open /dev/mem failed.\n");
		return -EBUSY;
	}

	/* MMAP MMIO: Alloc virtual memory and build Paging-Table */
	base = (char *)mmap(NULL, 
			    MMIO_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED,
			    fd,
			    MMIO_BASE);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	/* Don't trigger page-fault */
	val = (unsigned long *)base;
	*val = 88520;
	printf("%#lx => %ld\n", (unsigned long)val, *val);

	/* Detect physical information */
	if (!detect_physcial_address((unsigned long)base, &phys)) {
		printf("\nVaddr: %#lx\nPHYS:  %#lx\n", 
				(unsigned long)base, phys);
	}

	/* unmap */
	munmap(base, MMIO_SIZE);
	close(fd);

	return 0;
}
