/*
 * BiscuitOS Common system call: Strings paramenter
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
	char buffer[128];
	int nr;
};

/*
 * SYSCALL_DEFINE5(): Strings paramenter
 */
SYSCALL_DEFINE5(hello_BiscuitOS,
			char, ch,
			char __user *, const_strings,
			char __user *, string_array,
			char __user *, string_ptr,
			struct BiscuitOS_node __user *, string_struct)
{
	char kconst_strings[128];
	const char *str0 = "Kernel-BiscuitOS V5.0";
	const char *str1 = "Kernel-hello BiscuitOS";
	const char *str2 = "Kernel-BiscuitOS Plus";

	/* Copy string data from Userspace */
	if (copy_from_user(kconst_strings, const_strings, 128)) {
		printk("Const string copy error.\n");
		return -EINVAL;
	}
	printk("BiscuitOS_Kernel-char: %c\n", ch);
	printk("BiscuitOS_Kernel-constring: %s\n", kconst_strings);

	/* Copy kernel string to userspace */
	if (copy_to_user(string_array, str0, strlen(str0) + 1)) {
		printk("Copy string array failed.\n");
		return -EINVAL;
	}

	if (copy_to_user(string_ptr, str1, strlen(str1) + 1)) {
		printk("Copy string ptr failed.\n");
		return -EINVAL;
	}

	if (copy_to_user(string_struct->buffer, str2, strlen(str2) + 1)) {
		printk("Copy string struct failed.\n");
		return -EINVAL;
	}

	return 0;
}
