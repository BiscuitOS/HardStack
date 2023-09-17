// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Anonymous Memory on Stack
 *
 * (C) 2023.09.17 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	(8192)

int main()
{
	char *overflow;
	char base[MAP_SIZE];

	/* Overflow Stack */
	overflow = base + MAP_SIZE * 2 - 1;
	printf("Stack: %#lx\n", (unsigned long)overflow);

	/* Write Ops, Trigger #PF with Stack */
	*overflow = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("Stack %#lx => %c\n", (unsigned long)base, *base);

	return 0;
}
