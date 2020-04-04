/*
 * rculock
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
#include <linux/rcupdate.h>
#include <linux/list.h>

/*
 * return the ->next pointer of a list_head in an rcu safe
 * way, we must not access it directly
 */
#define list_next_rcu(list)	(*((struct list_head __rcu **)(&(list)->next)))

/*
 * Insert a new entry between two know consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add_rcu(struct list_head *new,
		struct list_head *prev, struct list_head *next)
{
	new->next = next;
	new->prev = prev;
	rcu_assign_pointer(list_next_rcu(prev), new);
	next->prev = new;
}

static inline void list_add_rcu(struct list_head *new, struct list_head *head)
{
	__list_add_rcu(new, head, head->next);
}

static inline void list_del_rcu(struct list_head *entry)
{
	__list_del_entry(entry);
	entry->prev = LIST_POISON2;
}

/*
 * list_entry_rcu - get the struct for this entry
 */
#define list_entry_rcu(ptr, type, member)				\
	container_of(READ_ONCE(ptr), type, member)

/*
 * list_for_each_entry_rcu - iterate over rcu list of given type
 */
#define list_for_each_entry_rcu(pos, head, member)			\
	for (pos = list_entry_rcu((head)->next, typeof(*pos), member);	\
		&pos->member != (head);					\
		pos = list_entry_rcu(pos->member.next, typeof(*pos), member))

/*
 * list_replace_rcu - replace old entry by new one
 */
static inline void list_replace_rcu(struct list_head *old,
					struct list_head *new)
{
	new->next = old->next;
	new->prev = old->prev;
	rcu_assign_pointer(list_next_rcu(new->prev), new);
	new->next->prev = new;
	old->prev = LIST_POISON2;
}

struct node {
	int index;
	struct list_head list;
};

static struct node node0 = { .index = 0x98 };
static struct node node1 = { .index = 0x48 };
static struct node node2 = { .index = 0x38 };
static struct node node3 = { .index = 0x68 };
static struct node node4 = { .index = 0x88 };

/* list head */
LIST_HEAD(Header);

/* Spinlock */
DEFINE_SPINLOCK(spinlock);

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct node *tmp;

	/* add node */
	spin_lock(&spinlock);
	list_add_rcu(&node0.list, &Header);
	list_add_rcu(&node1.list, &Header);
	list_add_rcu(&node2.list, &Header);
	list_add_rcu(&node3.list, &Header);
	list_add_rcu(&node4.list, &Header);
	spin_unlock(&spinlock);

	/* rcu read lock */
	rcu_read_lock();
	list_for_each_entry_rcu(tmp, &Header, list)
		printk("INDEX1: %#x\n", tmp->index);
	/* rcu read unlock */
	rcu_read_unlock();
	
	/* del node */
	spin_lock(&spinlock);
	list_del_rcu(&node0.list);
	list_del_rcu(&node1.list);
	spin_unlock(&spinlock);
	printk("Del: %#x - %#x\n", node0.index, node1.index);

	spin_lock(&spinlock);
	list_replace_rcu(&node2.list, &node0.list);
	spin_unlock(&spinlock);
	/* sync */
	synchronize_rcu();
	printk("Replace %#x to %#x\n", node2.index, node0.index);

	/* rcu read lock */
	rcu_read_lock();
	list_for_each_entry_rcu(tmp, &Header, list)
		printk("INDEX2: %#x\n", tmp->index);
	/* rcu read unlock */
	rcu_read_unlock();

	/* sync */
	synchronize_rcu();

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
MODULE_DESCRIPTION("Common rculock device driver");
