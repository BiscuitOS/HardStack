/*
 * hlist_bl
 *
 * (C) 2019.11.14 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list_bl.h>
#include <linux/hash.h>
#include <linux/rculist_bl.h>

#define DEMO_HLIST_SHIFT	10
#define DEMO_HASH_RANDOM	0x20

struct Demo_node {
	struct hlist_bl_node node;
	int idx;
};

struct Demo_node node0 = { .idx = 0x01, };

static struct hlist_bl_head Demo_hashtable[1 << DEMO_HLIST_SHIFT];

/* Hash */
static inline struct hlist_bl_head *Demo_hash(const void *arg, 
						unsigned int hash)
{
	hash += (unsigned long)arg * DEMO_HASH_RANDOM;
	return Demo_hashtable + hash_32(hash, DEMO_HLIST_SHIFT);
}

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct hlist_bl_head *head;
	struct hlist_bl_node *hnp;
	struct Demo_node *np;
	int i;

	/* Initialize hashlist_bl */
	for (i = 0; i < ARRAY_SIZE(Demo_hashtable); i++)
		INIT_HLIST_BL_HEAD(&Demo_hashtable[i]);

	/* add hlist_bl_node node 0 */
	head = Demo_hash(&node0, node0.idx);
	hlist_bl_lock(head);
	hlist_bl_add_head_rcu(&node0.node, head);

	/* Travel hlist */
	hlist_bl_for_each_entry(np, hnp, head, node)
		printk("IDX: %#x\n", np->idx);

	hlist_bl_unlock(head);

	/* delete node */
	head = Demo_hash(&node0, node0.idx);
	hlist_bl_lock(head);
	hlist_bl_del_init_rcu(&node0.node);

	/* Travel hlist */
	hlist_bl_for_each_entry(np, hnp, head, node)
		printk("DIDX: %#x\n", np->idx);
	hlist_bl_unlock(head);
	
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
MODULE_DESCRIPTION("Device driver");
