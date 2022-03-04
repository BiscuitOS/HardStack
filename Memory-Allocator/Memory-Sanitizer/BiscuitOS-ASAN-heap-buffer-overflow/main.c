/*
 * ASAN: heap-buffer-overflow (C Version)
 *
 * (C) 2022.02.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
	int *p = NULL;

	/* alloc heap memory */
	p = malloc(sizeof(int) * 8);

	/* BOOM: heap buffer overflow */
	p[8] = 0x20;

	/* free heap memory */
	free(p);

	return p[8];
}
