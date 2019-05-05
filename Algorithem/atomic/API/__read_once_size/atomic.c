/*
 * atomic
 *
 * (C) 2019.05.05 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* READ_ONCE/WRITE_ONCE */
#include <linux/compiler.h>

static __init int atomic_demo_init(void)
{
	volatile char ch = 'A';
	char cb;

	/* Read from memory not cache nor register */
	__read_once_size(&ch, &cb, sizeof(ch));

	printk("cb: %c\n", cb);

	return 0;
}
device_initcall(atomic_demo_init);
