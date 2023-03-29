// SPDX-License-Identifier: GPL-2.0
/*
 * FLUSH CACHE: CLFLUSH on USERSPACE
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <x86intrin.h>

int main()
{
	char *addr = malloc(64);

	sprintf(addr, "Hello BiscuitOS");

	/* Clear Flush */
	_mm_clflush(addr);

	printf("%s\n", addr);

	free(addr);

	return 0;
}
