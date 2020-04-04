/*
 * rwlock
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rwlock_types.h>

/* Declar and init rwlock */
DEFINE_RWLOCK(rwlock);

/* Module initialize entry */
static int __init Demo_init(void)
{
	unsigned long counter = 2;

	/* read lock */
	read_lock(&rwlock);
	printk("Read1: %ld\n", counter);
	/* read unlock */
	read_unlock(&rwlock);

	/* write lock */
	write_lock(&rwlock);
	counter++;
	/* write unlock */
	write_unlock(&rwlock);	

	/* read lock */
	read_lock(&rwlock);
	printk("Read2: %ld\n", counter);
	/* read unlock */
	read_unlock(&rwlock);

	return 0;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common rwlock device driver");
