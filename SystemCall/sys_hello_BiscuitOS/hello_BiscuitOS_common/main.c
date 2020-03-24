/*
 * BiscuitOS Common system call
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
/* Architecture defined */
#ifndef __NR_hello_BiscuitOS
/* ARM32 */
#ifdef __arm__
#define __NR_hello_BiscuitOS    400
/* ARM64 */
#elif __aarch64__
#define __NR_hello_BiscuitOS    401
/* Intel i386 */
#elif __i386__
#define __NR_hello_BiscuitOS    402
/* Intel X64 */
#elif __ia64__
#define __NR_hello_BiscuitOS    403
/* RISCV32 */
#elif __riscv32__
#define __NR_hello_BiscuitOS    404
/* RISCV64 */
#elif __riscv64__
#define __NR_hello_BiscuitOS    405
#endif
#endif

int main(void)
{
	printf("Hello: %d\n", __NR_hello_BiscuitOS);
	/*
	 * sys_hello_BiscuitOS
	 */
	syscall(__NR_hello_BiscuitOS);

	return 0;
}
