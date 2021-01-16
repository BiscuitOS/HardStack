/*
 * Nodemask:first_unset_node
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

        /* set all bits */
        nodes_setall(node_a);

        /* clear special bits */
        node_clear(12, node_a);

        /* first lowest clear bit */
        printk("The first clear bit: %d\n", first_unset_node(node_a));

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
