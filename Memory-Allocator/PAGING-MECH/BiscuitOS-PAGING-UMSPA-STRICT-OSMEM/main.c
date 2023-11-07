// SPDX-License-Identifier: GPL-2.0
/*
 * UMSPA: STRICT OSMEM
 *
 * (C) 2023.11.03 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define MAP_SIZE	0x1000

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
	if (fd < 0)
		errExit("Open /proc/self/pagemap failed.\n");

	if (lseek(fd, voffset, SEEK_SET) < 0)
		errExit("ERROR: lseek failed.\n");

	if (read(fd, &item, sizeof(uint64_t)) != sizeof(uint64_t))
		errExit("ERROR: read item error.\n");

	if ((((uint64_t)1 << 63) & item) == 0)
		errExit("Physical page doesn't alloc.\n");

	/* Obtain physical page information */
	pfn = (((uint64_t)1 << 55) - 1) & item;
	*paddr = (pfn * pagesize) + pgoff;
	close(fd);
	return 0;
}

int main()
{
	unsigned long phys;
	char *base, *addr;
	int fd;

	/* ALLOC VIRTUAL MEMORY */
	base = malloc(MAP_SIZE);
	if (!base)
		errExit("Alloc Virtual memory failed.\n");

	*base = 'B'; /* ALLOC PHYSICAL MEMORY */
	printf("UMSPA BEFORE READ: %#lx => %c\n", (unsigned long)base, *base);

	/* Detect physical information */
	if (!detect_physcial_address((unsigned long)base, &phys))
		printf("UMSP PHYS: %#lx\n", phys);

	/* OPEN FILE */
	fd = open("/dev/mem", O_RDWR);
	if (fd < 0)
		errExit("ERROR: Open /dev/mem failed.\n");

	/* UMSPA MAPPING NORMAL PHYSMEM */
	addr = (char *)mmap(NULL, MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED,
			    fd,
			    phys);
	if (addr == MAP_FAILED)
		errExit("ERROR: mmap failed.\n");

	/* ACCESS NORMAL PHYSMEM */
	*addr = 'D';

	/* CHECK */
	printf("UMSPA AFTER READ: %#lx => %c\n", (unsigned long)base, *base);

	/* RECLAIM */
	munmap(addr, MAP_SIZE);
	close(fd);
	free(base);

	return 0;
}
