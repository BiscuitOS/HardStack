/*
 * System Call: open
 *
 * (C) 2019.11.28 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE3(BiscuitOS_open, const char __user *, filename, 
			   int, flags, 
			   umode_t, mode)
{
	printk("hello BiscuitOS open\n");
	return 9;
}
