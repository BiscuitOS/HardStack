/*
 * Nodemask: node_online
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
        int nid = numa_node_id();

        /* NUMA Statistic */
        printk("The number of online nodes: %d\n", num_online_nodes());
        printk("The number of possible nodes: %d\n", num_possible_nodes());

        if (node_online(nid))
                printk("The NUMA NODE %d is online.\n", nid);
        if (node_possible(nid))
                printk("The NUMA NODE %d is possible online.\n", nid);

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
