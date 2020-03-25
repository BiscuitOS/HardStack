/*
 * BiscuitOS Common system call: Pointer paramenter
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
 * SYSCALL_DEFINE3(): Pointer paramenter
 */
SYSCALL_DEFINE3(hello_BiscuitOS,
			int __user *, int_ptr,
			char __user *, str_ptr,
			struct BiscuitOS_node __user *, node_ptr)
{
	char kbuffer[128];
	struct BiscuitOS_node knode;
	int kint;
	const char *str0 = "Kernel-BiscuitOS V5.0";
	const char *str1 = "Kernel-hello BiscuitOS";

	/* Copy string data from Userspace */
	if (copy_from_user(kbuffer, str_ptr, sizeof(char) * 128)) {
		printk("strings copy error.\n");
		return -EINVAL;
	}

	if (copy_from_user(&knode, node_ptr, sizeof(struct BiscuitOS_node))) {
		printk("Struct pointer copy error.\n");
		return -EINVAL;
	}

	if (copy_from_user(&kint, int_ptr, sizeof(int))) {
		printk("Integer pointer copy error.\n");
		return -EINVAL;
	}

	printk("BiscuitOS_Kernel-string:  %s\n", kbuffer);
	printk("BiscuitOS_Kernel-integer: %d\n", kint);
	printk("BiscuitOS_Kernel-node[0]: %s\n", knode.buffer);

	/* set */
	kint = 96;
	strcpy(knode.buffer, str0);
	strcpy(kbuffer, str1);

	/* Copy kernel data to userspace */
	if (copy_to_user(str_ptr, kbuffer, sizeof(char) * 128)) {
		printk("Copy string pointer failed.\n");
		return -EINVAL;
	}

	if (copy_to_user(int_ptr, &kint, sizeof(int))) {
		printk("Copy int ptr failed.\n");
		return -EINVAL;
	}

	if (copy_to_user(node_ptr, &knode, sizeof(struct BiscuitOS_node))) {
		printk("Copy struct failed.\n");
		return -EINVAL;
	}

	return 0;
}
