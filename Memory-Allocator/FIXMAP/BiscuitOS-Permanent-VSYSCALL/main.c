/*
 * Permanent Mapping Memory Allocator: VSYSCALL
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
#include <sys/time.h>

/* VSYSCALL: 0xFFFFFFFFFF600000 */
#define VSYSCALL_BASE		(-10UL << 20) 
/* Vsyscall page: 1st 1KiB for gettimeofdata */
#define VADDR_GETTIMEOFDAY	(VSYSCALL_BASE + 0x1000 * 0)

typedef int (*TOD_t)(struct timeval *tv, struct timezone *tz);

int main()
{
	struct timezone tz;
	struct timeval tv;

	TOD_t BiscuitOS_timeofday = (TOD_t)VADDR_GETTIMEOFDAY;
	BiscuitOS_timeofday(&tv, &tz);

	printf("Time: %d sec %d usec\n", (unsigned int)tv.tv_sec, 
					 (unsigned int)tv.tv_usec);

	return 0;
}
