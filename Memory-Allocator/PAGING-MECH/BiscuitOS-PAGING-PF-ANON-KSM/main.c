// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Anonymous on KSM
 *
 *   Enable CONFIG_KSM
 *
 * (C) 2023.09.14 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096)
#define MAP_VADDRA	(0x6000000000)
#define MAP_VADDRB	(0x7000000000)

static void *alloc_anonymous_memory(void *vaddr)
{
	return  mmap((void *)vaddr, MAP_SIZE, PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

int main()
{
	char *baseA, *baseB;

	baseA = alloc_anonymous_memory((void *)MAP_VADDRA);
	if (baseA == MAP_FAILED) {
		printf("ERROR: mmap A failed.\n");
		return -1;
	}
	baseB = alloc_anonymous_memory((void *)MAP_VADDRB);
	if (baseB == MAP_FAILED) {
		printf("ERROR: mmap B failed.\n");
		return -1;
	}

	/* Build KSM Page */
	memset(baseA, 0x00, MAP_SIZE);
	memset(baseB, 0x00, MAP_SIZE);
	sprintf(baseA, "Hello BiscuitOS");
	sprintf(baseB, "Hello BiscuitOS");

	/* Add KSM-Thread */
	madvise(baseA, MAP_SIZE, MADV_MERGEABLE);
	madvise(baseB, MAP_SIZE, MADV_MERGEABLE);

	printf("ANON-KSM: %#lx Wait KSM.\n", (unsigned long)baseA);
	sleep(1); /* Wakeup KSM-Thread on RunBiscuitOS.sh */

	/* Write Ops, Trigger #PF with KSM - COPY PAGE */
	*baseA = 'B';
	/* Write Ops, Trigger #PF with KSM - COPY PAGE */
	*baseB = 'D';

	sleep(-1); /* Just for Debug */
	munmap(baseA, MAP_SIZE);
	munmap(baseB, MAP_SIZE);

	return 0;
}
