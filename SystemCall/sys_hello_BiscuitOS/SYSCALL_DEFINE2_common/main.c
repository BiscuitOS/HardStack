/*
 * BiscuitOS Common system call: Two Paramenter
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
#ifndef __NR_hello_BiscuitOS
#define __NR_hello_BiscuitOS	400
#endif

int main(void)
{
	char buffer[128] = "BiscuitOS_Userspace";
	int ret;

	/*
	 * sys_hello_BiscuitOS: Two paramenter
	 * kernel:
	 *       SYSCALL_DEFINE2(hello_BiscuitOS,
	 *                       char __user *, strings,
	 *                       int, nr)
	 */
	ret = syscall(__NR_hello_BiscuitOS, buffer, strlen(buffer) + 1);
	if (ret < 0) {
		printf("SYSCALL failed %d\n", ret);
		return ret;
	} else
		printf("BiscuitOS: %s\n", buffer);

	return 0;
}
