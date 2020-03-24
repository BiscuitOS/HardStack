/*
 * BiscuitOS Common system call: One Paramenter
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
/* ARM32 */
#ifdef __arm__
#define __NR_hello_BiscuitOS    400
/* ARM64 */
#elif __aarch64__
#define __NR_hello_BiscuitOS    400
/* Intel i386 */
#elif __i386__
#define __NR_hello_BiscuitOS    387
/* Intel X64 */
#elif __x86_64__
#define __NR_hello_BiscuitOS    548
/* RISCV32 */
#elif __riscv_xlen == 32
#define __NR_hello_BiscuitOS    258
/* RISCV64 */
#elif __riscv_xlen == 64
#define __NR_hello_BiscuitOS    258
#endif
#endif

int main(void)
{
	char buffer[128] = "BiscuitOS_Userspace";
	int ret;

	/*
	 * sys_hello_BiscuitOS: One paramenter
	 * kernel:
	 *       SYSCALL_DEFINE1(hello_BiscuitOS,
	 *                       char __user *, strings)
	 */
	ret = syscall(__NR_hello_BiscuitOS, buffer);
	if (ret < 0) {
		printf("SYSCALL failed %d\n", ret);
		return ret;
	} else
		printf("BiscuitOS: %s\n", buffer);

	return 0;
}
