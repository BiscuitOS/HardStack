/*
 * Cgroup memory subsystem example
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
	printf("Cgroup memory subsystem example: %ld\n", (long)getpid());

	while (1) {
		malloc(4096);
		sleep(1);
	}

	return 0;
}
