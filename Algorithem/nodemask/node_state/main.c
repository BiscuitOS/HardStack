/*
 * Nodemask: node_state
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

        /* Check online */
        if (node_state(nid, N_ONLINE))
                printk("NUMA NODE %d has online.\n", nid);
        else
                printk("NUMA NODE %d has offline.\n", nid);

        /* Check possible online */
        if (node_state(nid, N_POSSIBLE))
                printk("NUMA NODE %d could become online at some point.\n", nid);

        /* Check normal momory */
        if (node_state(nid, N_NORMAL_MEMORY))
                printk("NUMA NODE %d has regular memory.\n", nid);

        /* Check Highmem memory */
        if (node_state(nid, N_HIGH_MEMORY))
                printk("NUMA NODE %d has regular or high memory.\n", nid);

        /* Check memory */
        if (node_state(nid, N_MEMORY))
                printk("NUMA NODE %d has memory (regular/high/movable)\n", nid);

        /* Check CPU */
        if (node_state(nid, N_CPU))
                printk("NUMA NODE %d has one or more CPUs.\n", nid);

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
