/*
 * Hardware Offline Poison Page.
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>

#define BISCUITOS_MAP_SIZE	4096
#define PAGEMAP_FILE		"/proc/self/pagemap"

static int detect_physical_address(unsigned long vaddr, unsigned long *paddr)
{
	int pagesize = getpagesize();
	unsigned long vpage_index;
	unsigned long pfn;
	unsigned long voffset;
	unsigned long pgoff;
	uint64_t item = 0;
	int fd;

	vpage_index = vaddr / pagesize;
	voffset = vpage_index * sizeof(uint64_t);
	pgoff = vaddr % pagesize;

	fd = open(PAGEMAP_FILE, O_RDONLY);
	if (fd < 0) {
		printf("ERROR: open %s Failed.\n", PAGEMAP_FILE);
		return -EINVAL;
	}

	if (lseek(fd, voffset, SEEK_SET) == -1) {
		printf("ERROR: lseek failed.\n");
		close(fd);
		return -EINVAL;
	}

	if (read(fd, &item, sizeof(uint64_t)) != sizeof(uint64_t)) {
		printf("ERROR: Read failed.\n");
		close(fd);
		return -EIO;
	}

	if ((((uint64_t)1 << 63) & item) == 0) {
		printf("Page doesn't present!\n");
		close(fd);
		return -EINVAL;
	}

	pfn = (((uint64_t)1 << 55) - 1) & item;
	*paddr = (pfn * pagesize) + pgoff;
	close(fd);

	return 0;
}

static void record_physical_address(unsigned long paddr)
{
	char buffer[64];
	int fd = open("/tmp/.posion_paddr.txt", O_RDWR | O_CREAT);

	/* record physical on file */
	sprintf(buffer, "%#lx", paddr);
	write(fd, buffer, strlen(buffer));
	close(fd);
}

int main()
{
	unsigned long paddr;
	char *base;

	base = (char *)mmap(NULL, BISCUITOS_MAP_SIZE,
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				-1, 0);
	if (base == MAP_FAILED) {
		printf("Uncorrect Error: No free Virtual memory\n");
		return -ENOMEM;
	}

	/* Trigger page fault */
	*(char *)base = 'B';

	/* Detect physical address */
	if (detect_physical_address((unsigned long)base, &paddr) == 0) {
		printf("Virtual-Address:  %#lx\nPhysical-Address: %#lx\n",
				(unsigned long)base, paddr);
		/* Record Physical address */
		record_physical_address(paddr);
		/* Trigger offline page */
		sleep(-1);
	}

	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
