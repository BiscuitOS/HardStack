/*
 * BiscuitOS Debug system call
 *
 * (C) 2020.03.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/syscalls.h>

int bs_debug_enable;
EXPORT_SYMBOL(bs_debug_enable);

SYSCALL_DEFINE1(debug_BiscuitOS, int, enable)
{
	if (enable)
		bs_debug_enable = 1;
	else
		bs_debug_enable = 0;

	return 0;
}
