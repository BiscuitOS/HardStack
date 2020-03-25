/*
 * BiscuitOS Common system call: Pointer Paramenter
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
	struct BiscuitOS_node nodes = { .buffer = "Struct" };
	struct BiscuitOS_node *node_ptr = &nodes;
	char buffer[64] = "String";
	char *str_ptr = buffer;
	int data = 80;
	int *int_ptr = &data;
	int ret;

	/*
	 * sys_hello_BiscuitOS: Pointer paramenter
	 * kernel:
	 *       SYSCALL_DEFINE3(hello_BiscuitOS,
	 *                       int __user *, int_ptr,
	 *                       char __user *, char_ptr,
	 *                       struct BiscuitOS_node __user *, node_ptr);
	 */
	ret = syscall(__NR_hello_BiscuitOS, 
					int_ptr,
					str_ptr,
					node_ptr);
	if (ret < 0) {
		printf("SYSCALL failed %d\n", ret);
		return ret;
	}

	printf("Array-Integer: %d\n", *int_ptr);
	printf("Array-string:  %s\n", str_ptr);
	printf("Array-struct:  %s\n", node_ptr->buffer);

	return 0;
}
