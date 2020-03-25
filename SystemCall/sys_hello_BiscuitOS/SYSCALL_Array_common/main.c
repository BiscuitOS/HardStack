/*
 * BiscuitOS Common system call: Array Paramenter
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
#include <malloc.h>

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

struct BiscuitOS_node {
	char buffer[32];
	int nr;
};

int main(void)
{
	struct BiscuitOS_node nodes[8] = { { .buffer = "Struct" } };
	char buffer[64] = "String";
	int int_array[64] = { 23, 45, 67, 81 };
	int ret;

	/*
	 * sys_hello_BiscuitOS: Array paramenter
	 * kernel:
	 *       SYSCALL_DEFINE3(hello_BiscuitOS,
	 *                       int __user *, int_array,
	 *                       char __user *, char_array,
	 *                       struct BiscuitOS_node __user *, node_array);
	 */
	ret = syscall(__NR_hello_BiscuitOS, 
					int_array,
					buffer,
					nodes);
	if (ret < 0) {
		printf("SYSCALL failed %d\n", ret);
		return ret;
	}

	printf("Array-Integer:    %d-%d\n", int_array[0], int_array[63]);
	printf("Array-string:     %s\n", buffer);
	printf("Array-struct[0]:  %s\n", nodes[0].buffer);
	printf("Array-struct[7]:  %s\n", nodes[7].buffer);

	return 0;
}
