// SPDX-License-Identifier: GPL-2.0
/*
 * BiscuitOS CACHE
 *
 * (C) 2023.05.04 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <linux/mman.h>

#define HPAGE_SIZE		(1 * 1024 * 1024 * 1024)
#define PAGE_SIZE		(4 * 1024)
#define BISCUITOS_MAP_SIZE	(1 * HPAGE_SIZE)

#define LLC_SIZE		(16 * 1024 * 1024) /* LLC 16MiB */
#define LLC_WAY			16
#define CACHE_LINE_SIZE		64
#define LLC_CACHE_SET_NUMR	(LLC_SIZE / (LLC_WAY * CACHE_LINE_SIZE))
#define LLC_CACHE_SET_SIZE	(LLC_WAY * CACHE_LINE_SIZE)
#define CACHE_SET_PER_4K	(PAGE_SIZE / CACHE_LINE_SIZE)
#define LLC_CACHE_BIN_SIZE	(CACHE_SET_PER_4K * LLC_CACHE_SET_SIZE)
#define ARRAY_SIZE		(2 * LLC_WAY)
#define LOOP			1000

static char *Colour_base0[ARRAY_SIZE];
static char *Colour_base1[ARRAY_SIZE];
static unsigned long count, idx;

static inline void setup_colour(char **array, char *base, unsigned long offset)
{
	for (idx = 0; idx < ARRAY_SIZE; idx++)
		array[idx] = idx * offset + base;
}

static void performance_testing(char **array, const char *CASE)
{
	struct timeval tv, ntv;

	count = LOOP;
	gettimeofday(&tv, NULL);
	while (count--) {
		for (idx = 0; idx < ARRAY_SIZE; idx++)
			*array[idx] = 'a' + idx;
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
			    MAP_SHARED | MAP_ANONYMOUS | 
			    MAP_HUGETLB | (30 << 26) /* MAP_HUGE_1GB */,
			    -1,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	setup_colour(Colour_base0, base, LLC_CACHE_BIN_SIZE);
	setup_colour(Colour_base1, base + PAGE_SIZE, CACHE_LINE_SIZE);

	performance_testing(Colour_base0, "Non-Colouring");
	performance_testing(Colour_base1, "Colouring    ");

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);

	return 0;
}
