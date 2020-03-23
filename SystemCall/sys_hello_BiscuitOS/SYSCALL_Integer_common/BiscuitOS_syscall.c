/*
 * BiscuitOS Common system call: Integer paramenter
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
 * SYSCALL_DEFINE6(): Integer paramenter
 */
SYSCALL_DEFINE6(hello_BiscuitOS,
			bool, enable,
			char, integer_char,
			short, integer_short,
			int, integer_int,
			unsigned long, integer_long,
			unsigned long long, integer_llong) 
{
	printk("Integer-bool:\t%s\n", enable == true ? "true" : "false");
	printk("Integer-char:\t%c\n", integer_char);
	printk("Integer-short:\t%#hhx\n", integer_short);
	printk("Integer-int:\t%#x\n", integer_int);
	printk("Integer-long:\t%#lx\n", integer_long);
	printk("Integer-llong:\t%#llx\n", integer_llong);

	return 0;
}
