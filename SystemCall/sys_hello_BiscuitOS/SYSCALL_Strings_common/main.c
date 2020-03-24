/*
 * BiscuitOS Common system call: Strings Paramenter
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
	char buffer[128];
	int nr;
};

int main(void)
{
	const char *strings = "Hello BiscuitOS";
	char *buffer[128];
	char ch = 'B';
	char *ptr;
	struct BiscuitOS_node node = {
		.nr = 20,
	};
	int ret;

	/* malloc */
	ptr = (char *)malloc(128);

	/*
	 * sys_hello_BiscuitOS: String paramenter
	 * kernel:
	 *       SYSCALL_DEFINE5(hello_BiscuitOS,
	 *                       char, ch,
	 *                       char __user *, const_string,
	 *                       char __user *, string_array,
	 *                       char __user *, string_ptr,
	 *                       struct BiscuitOS_node __user *, string_struct);
	 */
	ret = syscall(__NR_hello_BiscuitOS, 
					ch, 
					strings,
					buffer,
					ptr,
					&node);
	if (ret < 0) {
		printf("SYSCALL failed %d\n", ret);
		return ret;
	}

	printf("String-array:\t%s\n", buffer);
	printf("String-ptr:\t%s\n", ptr);
	printf("String-struct:\t%s\n", node.buffer);

	free(ptr);

	return 0;
}
