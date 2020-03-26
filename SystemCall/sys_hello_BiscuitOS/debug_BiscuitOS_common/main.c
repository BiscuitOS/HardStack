/*
 * BiscuitOS Debug system call
 *
 * (C) 2020.03.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
/* __NR_* */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_debug_BiscuitOS
#define __NR_debug_BiscuitOS    400
#endif

int main(void)
{
	/*
	 * Enable debug
	 */
	syscall(__NR_debug_BiscuitOS, 1);


	/*
	 * Disable debug
	 */
	syscall(__NR_debug_BiscuitOS, 0);

	return 0;
}
