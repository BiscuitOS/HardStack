// SPDX-License-Identifier: GPL-2.0
/*
 * Trigger OOM with CGROUP
 #
 * (C) 2023.05.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE		(4096)

int main()
{
	char *mem;

	while (1) {
		mem = malloc(PAGE_SIZE);
		memset(mem, 0, PAGE_SIZE);
	}

	return 0;
}
