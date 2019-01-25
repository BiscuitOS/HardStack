/*
 * Hash-list: hlist_for_each_entry
 *
 * #define hlist_for_each_entry(pos,head,member)    \
 *     for (pos = hlist_entry_safe((head)->first,typeof(*(pos)),member); \
 *          pos; \
 *          pos = hlist_entry_safe((pos)->member.next,typeof(*(pos)),member))
 *
 * (C) 2019.01.25 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Hash-list
 *
 * +------------+
 * | hlist_head |
 * +------------+      +------------+      +------------+
 * | hlist_head |<---->| hlist_node |<---->| hlist_node |
 * +------------+      +------------+      +------------+
 * | hlist_head |
 * +------------+   
 * | .......... |
 * +------------+      +------------+
 * | hlist_head |<---->| hlist_node |
 * +------------+      +------------+
 * | hlist_head |
 * +------------+      +------------+      +------------+
 * | hlist_head |<---->| hlist_node |<---->| hlist_node |
 * +------------+      +------------+      +------------+
 * | hlist_head |
 * +------------+   
 * | hlist_head |
 * +------------+   
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* header of list */
#include <linux/list.h>
#include <linux/hash.h>

#define HASHTABLE_SIZE   10
#define HASH_KEY         63

/* private structure */
struct node {
    const char *name;
    struct hlist_node node;
};

/* Initialize a group node structure */
static struct node node0 = { .name = "uboot",     };
static struct node node1 = { .name = "kernel",    };
static struct node node2 = { .name = "rootfs",    };
static struct node node3 = { .name = "image",     };
static struct node node4 = { .name = "SPI",       };
static struct node node5 = { .name = "EEPROM",    };
static struct node node6 = { .name = "BiscuitOS", };

/* Declear and implement a hash list */
static struct hlist_head hashtable[HASHTABLE_SIZE];

/* calculate hash-key */
static int hash(struct node *node)
{
    return hash_long((unsigned long)node->name[0] + 
                     (unsigned long)node->name[3], HASH_KEY) % 
                                                   HASHTABLE_SIZE;
}

static __init int hashlist_demo_init(void)
{
    struct node *np;
    int idx;

    /* add a new entry on back */
    hlist_add_head(&node0.node, &hashtable[hash(&node0)]);
    hlist_add_head(&node1.node, &hashtable[hash(&node1)]);
    hlist_add_head(&node2.node, &hashtable[hash(&node2)]);
    hlist_add_head(&node3.node, &hashtable[hash(&node3)]);
    hlist_add_head(&node4.node, &hashtable[hash(&node4)]);
    hlist_add_head(&node5.node, &hashtable[hash(&node5)]);
    hlist_add_head(&node6.node, &hashtable[hash(&node6)]);

    /* Traverser all node on hash-list */
    for (idx = 0; idx < HASHTABLE_SIZE; idx++) {
        printk("HASH Table[%d]: ", idx);
        hlist_for_each_entry(np, &hashtable[idx], node)
            printk("-> %s", np->name);
        printk("\n");
    }

    return 0;
}
device_initcall(hashlist_demo_init);
