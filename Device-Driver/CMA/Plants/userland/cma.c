/*
 * Contiguous Memory Allocate Debris Application
 *
 * (C) 2019.10.08 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdarg.h>
/* random number */
#include <time.h>

#define CMA_MEM_ALLOCATE	_IOW('m', 1, unsigned int)
#define CMA_MEM_RELEASE		_IOW('m', 2, unsigned int)
#define CMA_PATH		"/dev/BiscuitOS_CMA"

/* CMA area */
struct CMA_demo_region {
	unsigned long virt;
	unsigned long phys;
	unsigned long offset;
	unsigned long length;
};

/* TestCase: Size map 
 *      1 --> 4K
 *      2 --> 8K
 *      3 --> 12K
 */
#define MAP_SIZE	256
#define CMA_TEST_SIZE	4096
static unsigned long size_map[MAP_SIZE] = {
	1,   1,   1,   4,   5,   6,   3,   4,   5,   6,   3,
	8,   6,  12,  24,  14,   1,   1,   4,  13,  24,  45,
	3,  15,  17,  15,  20,  23,  31,   1,  16,  12,   3,
	1,  31,   3,   4,  23,   4,   1,   1,   4,  36,  51,
	2,  16,  32,   2,   4,  34,   2,   4,  32,   1,   5,
	1,  13,  21,  17,   4,   1,   1,   3,   4,   6,  12,
	5,   2,   2,   1,   4,   4,  15,   7,   8,   9,  32,
	2,  12,   5,   5,   5,   4,  32,   2,  16,  23,  12,
	2,   1,   1,   1,   1,   3,   2,   4,   5,   5,   6,
	7,   8,   8,   8,   7,  16,  24,   5,  37,  15,  23,   /* 110 */
	4,  13,  34,   5,   2,  24,  26,  15,  14,  32,  12,
	2,   4,  34,   4,  12,  42,   1,   1,   1,   3,  31,
	1,   3,  34,   4,   4,   4,   4,  24,   4,   4,   4,
	4,   6,  32,  12,  34,   2,  21,  21,   3,   5,  21,   /* 154 */
	1,  12,   4,   4,   5,   3,  12,   7,   4,   7,   8,
	9,   1,   1,   3,   3,  16,  16,   3,   2,   2,  15,
	6,   4,   2,   2,   2,   1,   3,   4,   4,   5,   2,
	7,   7,   7,   5,  12,   3,   4,   5,   6,  34,   2,
	1,   1,   1,   1,   1,   1,   4,   4,   4,   4,   5,
	7,   5,   4,   3,   4,   2,   4,   8,   2,   1,   3,
	8,   4,   2,   1,   3,  23,   3,   3,   4,   1,   1,   /* 231 */
	3,   1,   3,   4,   4,   4,   4,   5,   5,   2,   1,
	2,   2,  12,  15,  31,   1,   2,   4,   5,   6,   3,
	8,   23                                                /* 255 */
};

/* CMA region array */
static struct CMA_demo_region region_array[MAP_SIZE];
/* CMA allocate counter */
static unsigned long alloc_counter = 0;
static unsigned long free_counter = 0;

int main()
{
	struct CMA_demo_region *region;
	void *base;
	int fd, index;

	fd = open(CMA_PATH, O_RDWR, 0);
	if (fd < 0) {
		printf("Can't open %s\n", CMA_PATH);
		return -1;
	}
	/* Build random source */
	srand(time(NULL));

	/* Random allocate/free CMA region */
	while (1) {
		index = (rand() % (MAP_SIZE - 1));

		region = &region_array[index];

		if (region->phys == 0) {
			/* Current region is empty, and allocate CMA */
			region->length = size_map[index] * CMA_TEST_SIZE;

			/* Allocate memory from CMA */
			if (ioctl(fd, CMA_MEM_ALLOCATE, region) < 0) {
				printf("Error-alloc: Alloc %ld Free %ld\n",
					alloc_counter, free_counter);
				goto err_free;
			}
			/* Audit number for allocating */
			alloc_counter++;
		} else {
			/* Free memory from CMA */
			if (ioctl(fd, CMA_MEM_RELEASE, region) < 0) {
				printf("Error-free: Alloc %ld Free %ld\n",
					alloc_counter, free_counter);
				close(fd);
				return -1;
			}
			memset(region, 0, sizeof(struct CMA_demo_region));
			/* Audit number for freeing */
			free_counter++;
		}

		/* delay */
	}

	close(fd);

	return 0;

err_free:
	for (index = 0; index < MAP_SIZE; index++) {
		region = &region_array[index];
		if (region->phys != 0)
			ioctl(fd, CMA_MEM_RELEASE, region);
	}
	close(fd);
	return -1;
}
