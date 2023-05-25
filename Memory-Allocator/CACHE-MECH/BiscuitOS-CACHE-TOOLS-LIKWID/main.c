// SPDX-License-Identifier: GPL-2.0
/*
 * CACHE TOOLs: Likwid
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/mman.h>
#include <likwid.h>

#define BLOCKS_SIZE	(4 * 1024) /* 64KiB Data */
#define LINES		64
#define BLOCKS		(BLOCKS_SIZE / LINES)

/* Likwid */
LIKWID_MARKER_INIT;

static char (*array)[BLOCKS];

int main()
{
	void *addr;
	int i, j;
	int count = 1000;

	addr = mmap(NULL, BLOCKS_SIZE, 
	      	   PROT_READ | PROT_WRITE,
		   MAP_SHARED | MAP_ANONYMOUS,
		   -1,
		   0);
	if (addr == MAP_FAILED) {
		printf("mmap failed.\n");
		return -1;
	}
	array = (char (*)[BLOCKS])addr;

	LIKWID_MARKER_START("BiscuitOS-CACHE-TOOLS");
	while (count--) {
		for (i = 0; i < BLOCKS; i++) {
			for (j = 0; j < LINES; j++) {
				/* Read A CACHE Line */
				char a = array[i][j];
			}
		}
	}
	LIKWID_MARKER_STOP("BiscuitOS-CACHE-TOOLS");
	
	munmap(addr, BLOCKS_SIZE);

	/* Stop monitor */
	LIKWID_MARKER_CLOSE;
	return 0;
}
