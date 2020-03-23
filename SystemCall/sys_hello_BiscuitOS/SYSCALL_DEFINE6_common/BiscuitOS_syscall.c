/*
 * BiscuitOS Common system call: Six paramenter
 *
 * (C) 2020.03.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/syscalls.h>

struct BiscuitOS_node {
	int index;
	int offset;
};

/*
 * SYSCALL_DEFINE6(): Six paramenter
 */
SYSCALL_DEFINE6(hello_BiscuitOS, char __user *, strings, 
			int, nr_write, int, nr_read, int __user *, reader,
			struct BiscuitOS_node __user *, bnode,
			int __user *, array)
{
	char buffer[128];
	const char *kstring = "Kernel_BiscuitOS";
	int Kreader = 8;
	struct BiscuitOS_node knode = {
		.index = 90,
		.offset = 99,
	};
	int karray[4] = { 23, 67, 89, 34};

	/* Copy string from userspace */
	if (copy_from_user(buffer, strings, nr_write)) {
		printk("copy_from_user() error\n");
		return -EINVAL;
	}

	printk("Hello BiscuitOS: %s\n", buffer);

	/* Copy kernel string to userspace */
	if (copy_to_user(strings, kstring, nr_read)) {
		printk("copy_to_user() error\n");
		return -EINVAL;
	}

	/* Copy kread to userspace */
	if (copy_to_user(reader, &Kreader, sizeof(int))) {
		printk("Reader copy failed\n");
		return -EINVAL;
	}

	/* Copy struct to userspace */
	if (copy_to_user(bnode, &knode, sizeof(struct BiscuitOS_node))) {
		printk("BiscuitOS_node error\n");
		return -EINVAL;
	}

	/* Copy array to userspace */
	if (copy_to_user(array, karray, sizeof(int) * 4)) {
		printk("Copy arrary error.\n");
		return -EINVAL;
	}

	return 0;
}
