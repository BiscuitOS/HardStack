// SPDX-License-Identifier: GPL-2.0
/*
 * CLEAR_REFS: HIWATER_RSS
 *
 * (C) 2023.08.30 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE	(4096)
#define MAP_SIZE	(20 * PAGE_SIZE)
#define MAP_VADDR	(0x6000000000)

int main()
{
	char *base;

	base = mmap((void *)MAP_VADDR, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	while (1) {
		int set = (random() % 19) * PAGE_SIZE;
		int clr = (random() % 19) * PAGE_SIZE;

		/* ADD RSS */
		base[set] = 'A';
		/* SUB RSS */
		if (base[clr] == 'A')
			madvise(&base[clr], PAGE_SIZE, MADV_DONTNEED);
		/* LOOP */
		sleep(1);
	}

	munmap(base, MAP_SIZE);

	return 0;
}
