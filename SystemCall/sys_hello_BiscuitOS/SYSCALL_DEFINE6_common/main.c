/*
 * BiscuitOS Common system call: Six Paramenter
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

struct BiscuitOS_node {
	int index;
	int offset;
};

int main(void)
{
	struct BiscuitOS_node bnode;
	char buffer[128] = "Userspace_BiscuitOS";
	int array[4];
	int nr_write = strlen(buffer) + 1;
	int nr_read = 6;
	int reader;
	int ret;

	/*
	 * sys_hello_BiscuitOS: Four paramenter
	 * kernel:
	 *       SYSCALL_DEFINE6(hello_BiscuitOS,
	 *                       char __user *, strings,
	 *                       int, nr_write,
	 *                       int, nr_read,
	 *                       int __user *, reader,
	 *                       struct BiscuitOS_node __user *, bnode,
	 *                       int __user *, array)
	 */
	ret = syscall(__NR_hello_BiscuitOS, buffer, 
				nr_write, nr_read, &reader, &bnode, array);
	if (ret < 0) {
		printf("SYSCALL failed %d\n", ret);
		return ret;
	} else {
		buffer[nr_read] = '\0';
		printf("BiscuitOS[%d]: %s\n", reader, buffer);
		printf("Bnode: %d - %d\n", bnode.index, bnode.offset);
		printf("Array: %d - %d - %d - %d\n", 
				array[0], array[1], array[2], array[3]);
	}

	return 0;
}
