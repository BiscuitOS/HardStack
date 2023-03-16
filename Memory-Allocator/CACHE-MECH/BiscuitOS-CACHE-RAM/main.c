/*
 * Default Memory Type on RAM
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
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

int main()
{
	void *addr;

	/* Allocate memory */
	addr = malloc(128);
	if (!addr) {
		printf("Allocate Memory Failed.\n");
		return -1;
	}

	/* Use */
	sprintf((char *)addr, "Hello BiscuitOS");
	printf("%s\n", (char *)addr);

	/* Reclaim */
	free(addr);
	return 0;
}
