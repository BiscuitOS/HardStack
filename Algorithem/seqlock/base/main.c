/*
 * seqlock
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
#include <linux/seqlock.h>

/* Declar and init seqlock */
DEFINE_SEQLOCK(seqlock);

/* Module initialize entry */
static int __init Demo_init(void)
{
	unsigned seq;
	int counter = 0;

	/* read lock */
	do {
		seq = read_seqbegin(&seqlock);
		printk("Read: %d seq %d\n", counter, seq);
	} while (read_seqretry(&seqlock, seq));

	/* write lock */
	write_seqlock(&seqlock);
	counter++;
	write_sequnlock(&seqlock);

	/* read lock */
	do {
		seq = read_seqbegin(&seqlock);
		printk("Read: %d seq %d\n", counter, seq);
	} while(read_seqretry(&seqlock, seq)); 

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
MODULE_DESCRIPTION("Common Seqlock device driver");
