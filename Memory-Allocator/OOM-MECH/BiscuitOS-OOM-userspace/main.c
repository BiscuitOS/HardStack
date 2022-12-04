/*
 * OOM on Userspace (BiscuitOS+)
 *
 * (C) 2020.05.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

/* Min cgroup memory region 128KiB */
#define REGION_SIZE	(128 * 1024)

int main()
{
	char *base;

	/* grow up memory */
	while (1) {
		/* alloc virtual memory */
		base = malloc(REGION_SIZE);
		/* alloc physical memory */
		memset(base, 0xff, REGION_SIZE);
		/* only debug */
		sleep(1);
	}
	/* hold process */
	sleep(-1);

	return 0;
}
