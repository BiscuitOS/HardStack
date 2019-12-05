/*
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>

static struct hlist_bl_head *BiscuitOS_hashtable __read_mostly;
static __initdata unsigned long BiscuitOS_entries;
static int BiscuitOS_shift __read_mostly;

static int __init set_BiscuitOS_entries(char *str)
{
	if (!str)
		return 0;
	BiscuitOS_entries = simple_strtoul(str, &str, 0);
	return 1;
}
__setup("BiscuitOS_entries=", set_BiscuitOS_entries);

static inline struct hlist_bl_head *BiscuitOS_hash(unsigned int hash)
{
	return BiscuitOS_hashtable + (hash >> BiscuitOS_shift);
}

/* Module initialize entry */
static int __init Demo_init(void)
{
	/* Don't use HASH_EARLY on late stage */
	BiscuitOS_hashtable =
		alloc_large_system_hash("BiscuitOS cache",
					sizeof(struct hlist_bl_head),
					BiscuitOS_entries,
					13,
					HASH_ZERO,
					&BiscuitOS_shift,
					NULL,
					0,
					0);

	/* HASH */
	printk("HASH....\n");

	return 0;
}
early_initcall(Demo_init);
