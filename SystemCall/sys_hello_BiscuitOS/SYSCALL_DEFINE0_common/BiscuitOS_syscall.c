/*
 * BiscuitOS Common system call: Zero paramenter
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
 * SYSCALL_DEFINE0(): Zero Paramenter
 */
SYSCALL_DEFINE0(hello_BiscuitOS)
{
	printk("Hello BiscuitOS: Zero Paramenter\n");

	return 0;
}
