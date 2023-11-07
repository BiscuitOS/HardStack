// SPDX-License-Identifier: GPL-2.0
/*
 * PAGING Project: Simpler ALLOCATOR
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
	char *buffer;

	/* ALLOC MEMORY */
	buffer = malloc(4096);
	if (!buffer)
		exit (-1);

	/* Trigger #PF */
	sprintf(buffer, "Hello BiscuitOS");
	printf("%s\n", buffer);

	/* RECLAIM */
	free(buffer);

	return 0;
}
