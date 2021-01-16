/*
 * Nodemask: num_node_state
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
        printk("Number of node:\n");
        printk("Online-Node: %d\n", num_node_state(N_ONLINE));
        printk("Possible Online: %d\n", num_node_state(N_POSSIBLE));
        printk("Normal Memory: %d\n", num_node_state(N_NORMAL_MEMORY));
        printk("High/Normal Memory: %d", num_node_state(N_HIGH_MEMORY));
        printk("Memory: %d\n", num_node_state(N_MEMORY));
        printk("CPU: %d\n", num_node_state(N_CPU));

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
