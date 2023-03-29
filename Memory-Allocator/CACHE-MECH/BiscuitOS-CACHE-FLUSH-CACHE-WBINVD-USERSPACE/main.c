// SPDX-License-Identifier: GPL-2.0
/*
 * FLUSH CACHE: WBINVD on USERSPACE
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <x86intrin.h>
#include <immintrin.h>

int main()
{
	char *addr = malloc(64);

	sprintf(addr, "Hello BiscuitOS");

	/* Write-Back and Invalid all */
	_wbinvd();

	printf("%s\n", addr);

	free(addr);

	return 0;
}
