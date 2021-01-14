/*
 * Nodemask: nodes_subset
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
        nodemask_t node_c;

        /* Clear all bits */
        nodes_clear(node_a);
        nodes_clear(node_b);
        nodes_clear(node_c);

        /* Set special bits */
        node_set(2, node_a);
        node_set(4, node_a);
        node_set(2, node_b);
        node_set(1, node_c);

        if (nodes_subset(node_b, node_a))
                printk("B is subset of A.\n");

        if (!nodes_subset(node_c, node_a))
                printk("C is not subset of A\n");

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
