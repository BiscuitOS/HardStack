/*
 * Hugetlb: Anonymous Shared-mmaping hugetlb on libhugetlbfs
 *
 * (C) 2021.11.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifdef __i386__
#error "Process doesn't support I386 Architecture"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

int main()
{
	char *buf;

	/* Allocate memory from libhugetlbfs */
	buf = malloc(0x400000);
	if (!buf) {
		printf("ERROR: alloc hugetlbfs failed.\n");
		return -1;
	}

	/* Use memory: Trigger page fault  */
	sprintf(buf, "BiscuitOS-%s", "hugetlbfs");
	printf("Hello BiscuitOS on %s.\n", buf);

	/* Only Debug */
	sleep(-1);

	/* Reclaim memory */
	free(buf);
	return 0;
}
