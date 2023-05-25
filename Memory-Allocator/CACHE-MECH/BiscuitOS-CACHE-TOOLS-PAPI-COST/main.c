// SPDX-License-Identifier: GPL-2.0
/*
 * CACHE TOOLs: PAPI - papi_cost
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

int main()
{
	long long start_time, end_time, cost;
	int event_set = PAPI_NULL;
	int i, j, count = 1000;
	long long values[3];
	void *addr;

	/* Initialize PAPI library */
	if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
		printf("ERROR: PAPI Library initialize failed\n");
		exit(-1);
	}
	PAPI_create_eventset(&event_set);
	/* Add EVENT */
	PAPI_add_event(event_set, PAPI_L1_DCH);

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

	PAPI_start(event_set);

	start_time = PAPI_get_real_nsec();

	while (count--) {
		for (i = 0; i < BLOCKS; i++) {
			for (j = 0; j < LINES; j++) {
				/* Read A CACHE Line */
				char a = array[i][j];
			}
		}
	}

	end_time = PAPI_get_real_nsec();
//	papi_cost(event_set, &start_time, &end_time, &cost);

	PAPI_stop(event_set, NULL);
	PAPI_cleanup_eventset(event_set);
	PAPI_destroy_eventset(&event_set);
	PAPI_shutdown();

	munmap(addr, SIZE);

	return 0;
}
