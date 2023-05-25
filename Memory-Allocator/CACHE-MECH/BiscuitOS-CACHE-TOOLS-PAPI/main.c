// SPDX-License-Identifier: GPL-2.0
/*
 * CACHE TOOLs: PAPI
 *
 * (C) 2023.05.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/mman.h>
#include <papi.h>

#define SIZE		(4 * 1024) /* 64KiB Data */
#define LINES		64
#define BLOCKS		(SIZE / LINES)

static char (*array)[BLOCKS];
/* PAPI Counter Sample */
static int papi_counter[] = { PAPI_L1_TCM, PAPI_L2_TCM, PAPI_L3_TCM};

int main()
{
	int i, j, count = 1000;
	long long values[3];
	void *addr;

	/* Initialize PAPI library */
	if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
		printf("ERROR: PAPI Library initialize failed\n");
		exit(-1);
	}

	addr = mmap(NULL, SIZE, 
	      	   PROT_READ | PROT_WRITE,
		   MAP_SHARED | MAP_ANONYMOUS,
		   -1,
		   0);
	if (addr == MAP_FAILED) {
		printf("mmap failed.\n");
		return -1;
	}
	array = (char (*)[BLOCKS])addr;

	/* START Counters */
	if (PAPI_start_counters(papi_counter, 3) != PAPI_OK) {
		printf("ERROR: PAPI start counters failed.\n");
		exit(-1);
	}

	while (count--) {
		for (i = 0; i < BLOCKS; i++) {
			for (j = 0; j < LINES; j++) {
				/* Read A CACHE Line */
				char a = array[i][j];
			}
		}
	}

	/* STOP Counters */
	if (PAPI_stop_counters(values, 3) != PAPI_OK) {
		printf("ERROR: PAPI stop counters failed.\n");
		exit(-1);
	}
	
	printf("L1 CACHE: %lld hits, %lld misses\n", values[0],
						BLOCKS - values[0]);
	printf("L2 CACHE: %lld hits, %lld misses\n", values[1],
						BLOCKS - values[1]);
	printf("L3 CACHE: %lld hits, %lld misses\n", values[2],
						BLOCKS - values[2]);

	munmap(addr, SIZE);

	return 0;
}
