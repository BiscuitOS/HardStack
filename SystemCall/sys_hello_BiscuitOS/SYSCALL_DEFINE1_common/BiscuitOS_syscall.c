/*
 * BiscuitOS Common system call: One paramenter
 *
 * (C) 2020.03.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/syscalls.h>

/*
 * SYSCALL_DEFINE1(): One paramenter
 */
SYSCALL_DEFINE1(hello_BiscuitOS, char __user *, strings)
{
	char buffer[128];
	const char *kstring = "BiscuitOS_kernel";

	/* Copy string from userspace */
	if (copy_from_user(buffer, strings, 128)) {
		printk("copy_from_user() error\n");
		return -EINVAL;
	}

	printk("Hello BiscuitOS: %s\n", buffer);

	/* Copy kernel string to userspace */
	if (copy_to_user(strings, kstring, strlen(kstring) + 1)) {
		printk("copy_to_user() error\n");
		return -EINVAL;
	}

	return 0;
}
