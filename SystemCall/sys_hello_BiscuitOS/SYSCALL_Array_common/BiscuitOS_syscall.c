/*
 * BiscuitOS Common system call: Array paramenter
 *
 * (C) 2020.03.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/string.h>

struct BiscuitOS_node {
	char buffer[32];
	int nr;
};

/*
 * SYSCALL_DEFINE3(): Array paramenter
 */
SYSCALL_DEFINE3(hello_BiscuitOS,
			int __user *, int_array,
			char __user *, buffer,
			struct BiscuitOS_node __user *, nodes)
{
	char kbuffer[128];
	int kint_array[64];
	struct BiscuitOS_node knodes[8];
	const char *str0 = "Kernel-BiscuitOS V5.0";
	const char *str1 = "Kernel-hello BiscuitOS";
	const char *str2 = "Kernel-BiscuitOS Plus";

	/* Copy string data from Userspace */
	if (copy_from_user(kbuffer, buffer, sizeof(char) * 128)) {
		printk("Const string copy error.\n");
		return -EINVAL;
	}

	if (copy_from_user(kint_array, int_array, sizeof(int) * 64)) {
		printk("Integer array copy error.\n");
		return -EINVAL;
	}

	if (copy_from_user(knodes, nodes, sizeof(struct BiscuitOS_node) * 8)) {
		printk("Struct array copy error.\n");
		return -EINVAL;
	}

	printk("BiscuitOS_Kernel-string:  %s\n", kbuffer);
	printk("BiscuitOS_Kernel-integer: %d - %d - %d\n", kint_array[0],
						kint_array[1], kint_array[2]);
	printk("BiscuitOS_Kernel-node[0]: %s\n", knodes[0].buffer);

	/* set */
	kint_array[0]  = 98;
	kint_array[63] = 29;
	strcpy(knodes[0].buffer, str0);
	strcpy(knodes[7].buffer, str1);
	strcpy(kbuffer, str2);

	/* Copy kernel array to userspace */
	if (copy_to_user(buffer, kbuffer, sizeof(char) * 128)) {
		printk("Copy string array failed.\n");
		return -EINVAL;
	}

	if (copy_to_user(int_array, kint_array, sizeof(int) * 64)) {
		printk("Copy string ptr failed.\n");
		return -EINVAL;
	}

	if (copy_to_user(nodes, knodes, sizeof(struct BiscuitOS_node) * 8)) {
		printk("Copy string struct failed.\n");
		return -EINVAL;
	}

	return 0;
}
