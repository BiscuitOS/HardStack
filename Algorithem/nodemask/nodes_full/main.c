/*
 * Nodemask: nodes_full
 *
 * (C) 2021.01.10 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
        nodemask_t node_a;
        nodemask_t node_b;

        /* Clear all bits */
        nodes_clear(node_a);
        nodes_setall(node_b);

        /* Set special bits */
        node_set(2, node_a);

        if (nodes_full(node_b))
                printk("Node b is full.\n");

        if (!nodes_full(node_a))
                printk("Node a is not full.\n");

        printk("Hello modules on BiscuitOS\n");

        return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common Device driver on BiscuitOS");
