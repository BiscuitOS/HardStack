/*
 * Nodemask: node_clear_state
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

        /* Node clear state */
        node_clear_state(nid, N_CPU);

        /* Check CPU State */
        if (!node_state(nid, N_CPU))
                printk("NUMA NODE %d doesn't contain any CPU.\n", nid);

        /* Node set state */
        node_set_state(nid, N_CPU);

        /* Check CPU State */
        if (node_state(nid, N_CPU))
                printk("NUMA NODE %d contains one or more CPUs.\n", nid);

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
