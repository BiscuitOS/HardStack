/*
 * Thread_info and kernel stack
 *
 * (C) 2019.07.01 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* thread */
#include <linux/sched.h>

static __init int thread_demo_init(void)
{
	struct thread_info *info;
	unsigned long stack;

	/* Obtain current thread_info address */
	info = current_thread_info();

	/* Obtian stack address */
	stack = (__force unsigned long)info;
	stack += THREAD_SIZE;

	printk("thread_info AD: %#lx\n", (__force unsigned long)info);
	printk("stack AD: %#lx\n", current_stack_pointer);
	printk("thread_union end AD: %#lx\n", stack);

	return 0;
}
device_initcall(thread_demo_init);
