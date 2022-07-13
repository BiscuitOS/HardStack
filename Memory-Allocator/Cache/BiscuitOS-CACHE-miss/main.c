/*
 * Cache Hit/Miss 
 *
 * (C) 2022.07.12 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>

/*
 * CACHE information
 *
 * +-----------+------------+-----------+-----------+-----------------+
 * |  Item     | Cache-Size | Cache Set | Cache-Way | Cache Line Size |
 * +-----------+------------+-----------+-----------+-----------------+
 * | L1 DCache | 32KiB      | 64        | 8         | 64              |
 * +-----------+------------+-----------+-----------+-----------------+
 * | L1 ICache | 32KiB      | 64        | 8         | 64              |
 * +-----------+------------+-----------+-----------+-----------------+
 * | L2 Cache  | 4MiB       | 4096      | 16        | 64              |
 * +-----------+------------+-----------+-----------+-----------------+
 * | L3 Cache  | 16MiB      | 16384     | 16        | 64              |
 * +-----------+------------+-----------+-----------+-----------------+
 * 
 * Force on L1 Dcache.
 */
#define L1_CACHE_SIZE			(32 * 1024)
#define L2_CACHE_SIZE			(4  * 1024 * 1024)
#define L3_CACHE_SIZE			(16 * 1024 * 1024)
#define L1_CACHE_SET			(64)
#define L2_CACHE_SET			(4096)
#define L3_CACHE_SET			(16384)
#define L1_CACHE_WAY			(8)
#define L2_CACHE_WAY			(16)
#define L3_CACHE_WAY			(16)
#define CACHE_LINE_SIZE			(64)
/* Next Physical Address on same Cache Set */
#define ADDR_OFFSET			(L3_CACHE_SET * CACHE_LINE_SIZE)
#define HPAGE_SIZE			(1 * 1024 * 1024 * 1024) /* 1Gig */
#define MAP_SIZE			(HPAGE_SIZE)
#define GROUND_OFFSET			(L3_CACHE_SIZE)
/* 1Gig Hugetlbfs huge */
#ifndef MAP_HUGE_1GB
#define HUGETLB_FLAG_ENCODE_SHIFT	(26)
#define MAP_HUGE_1GB			(30 << HUGETLB_FLAG_ENCODE_SHIFT)
#endif

static int read_nsec(unsigned long addr, int index)
{
	unsigned long time_be, time_af;
	struct timespec be, af;
	char val;

	clock_gettime(CLOCK_REALTIME, &be);
	val = *(char *)addr;
	clock_gettime(CLOCK_REALTIME, &af);

	/* calculate time */
	time_be = be.tv_sec * 1000000000 + be.tv_nsec;
	time_af = af.tv_sec * 1000000000 + af.tv_nsec;

	printf("Dcache %02d [%#08lx] cost %ld nsec\n", 
					index, addr, time_af - time_be);
	return 0;
}

int main()
{
	int i, j, index;

	char *base = (char *)mmap(NULL, MAP_SIZE,
				  PROT_READ | PROT_WRITE,
				  MAP_SHARED | MAP_ANONYMOUS |
				  MAP_HUGE_1GB | MAP_POPULATE,
				  -1, 0);
	if (base == MAP_FAILED)
		exit(-1);

	for (i = 0; i < 4; i++) {
		j = i;
		if (i == 2)
			i = 1; /* Read from Cache on loop-1 */
		printf("===============%d==============\n", i);
		for (index = 0; index < L3_CACHE_WAY + 2; index++)
			read_nsec((unsigned long)&base[ADDR_OFFSET *
					index + i * GROUND_OFFSET], index);
		i = j;
	}

	munmap(base, MAP_SIZE);
	return 0;
}
