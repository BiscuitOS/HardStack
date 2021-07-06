/*
 * CRASH: vtop
 *
 * (C) 2021.06.06 BuddyZhang1 <buddy.zhang@aliyun.com>
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
	void *address;

	address = malloc(40);
	printf("Virtual address: %#lx\n", (unsigned long)address);

	/* Loop for CRASH */
	while (1);

	free(address);

	return 0;
}
