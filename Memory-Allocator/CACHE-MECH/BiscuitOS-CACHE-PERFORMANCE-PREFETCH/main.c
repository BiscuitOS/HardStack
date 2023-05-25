// SPDX-License-Identifier: GPL-2.0
/*
 * Performance: Prefetch
 *
 * (C) 2023.05.04 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <linux/mman.h>

#define HPAGE_SIZE		(2 * 1024 * 1024)
#define PAGE_SIZE		(4 * 1024)
#define BISCUITOS_MAP_SIZE	(1 * HPAGE_SIZE)

#define ARRAY_SIZE		64
#define CACHE_LINE_SIZE		64
#define LOOP			10000

static char *Colour_base0[ARRAY_SIZE];
static char *Colour_base1[ARRAY_SIZE];
static unsigned long count, idx;

static inline void prefetch(const char *p)
{
	__asm__ volatile ("prefetch (%0)" :: "r" (p));
}

static inline void setup_array(char **array, char *base, unsigned long offset)
{
	for (idx = 0; idx < ARRAY_SIZE; idx++)
		array[idx] = idx * offset + base;
}

static void performance_testing(char **array, const char *CASE, bool pre)
{
	struct timeval tv, ntv;

	count = LOOP;
	gettimeofday(&tv, NULL);
	while (count--) {
		for (idx = 0; idx < ARRAY_SIZE; idx++) {
			*array[idx] = 'a' + idx;
			if (pre)
				prefetch(array[idx + 1]);
		}
	}
	gettimeofday(&ntv, NULL);
	printf("%s Cost Time: %ld nsec\n", CASE, ntv.tv_sec * 1000000 +
			ntv.tv_usec - tv.tv_sec * 1000000 - tv.tv_usec);
}

int main()
{
	char *base;

	/* mmap */
	base = (char *)mmap(NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE |  
			    MAP_HUGETLB | (21 << 26) /* MAP_HUGE_2MiB */,
			    -1,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	setup_array(Colour_base0, base, CACHE_LINE_SIZE);
	setup_array(Colour_base1, base + 16 * PAGE_SIZE, CACHE_LINE_SIZE);

	performance_testing(Colour_base0, "No-Prefetch", false);
	performance_testing(Colour_base1, "Prefetching", true);

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
