/*
 * NULL: Direct refer NULL pointer
 *
 * (C) 2021.05.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

int main()
{
	int *p = NULL;

	mmap(NULL, 4096, PROT_READ | PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

	*p = 88520;
	printf("%#lx=>%ld\n", p, *p);

	/* only debug */
	printf("PID %ld\n", (long)getpid());
	sleep(25);

	return 0;
}
