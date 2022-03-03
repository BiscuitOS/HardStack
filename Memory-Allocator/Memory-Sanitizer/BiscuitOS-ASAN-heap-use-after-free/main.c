/*
 * ASAN: Heap use after free
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

int func(void)
{
	int *p = NULL;

	/* alloc heap memory */
	p = (int *)malloc(sizeof(int));
	/* free heap memory */
	free(p);

	/* Use after free */
	return *p;
}

int main()
{
	int p = func();

	return 0;
}
