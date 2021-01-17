/*
 * Nodemask: node_set_offline
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

        /* Offline NUMA NODE */
        node_set_offline(nid);

        /* Check NUMA NODE state */
        if (!node_state(nid, N_ONLINE))
                printk("Warning NUMA NODE %d offline!\n", nid);

        /* Online NUMA NODE */
        node_set_online(nid);

        /* Check NUMA NODE state */
        if (node_state(nid, N_ONLINE))
                printk("NUMA NODE %d online!\n", nid);

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
