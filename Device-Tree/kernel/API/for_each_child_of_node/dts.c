/*
 * DTS: of_get_child_count
 *      for_each_child_of_node
 *      for_each_available_of_node
 *
 * static inline int of_get_child_count(const struct device_node *np)
 *
 * #define for_each_child_of_node(parent, child) \
 *         for (child = of_get_next_child(parent, NULL); child != NULL; \
 *              child = of_get_next_child(parent, child))
 *
 * #define for_each_available_child_of_node(parent, child) \
 *     for (child = of_get_next_available_child(parent, NULL); child != NULL; \
 *          child = of_get_next_available_child(parent, child))
 *
 * (C) 2019.01.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Private DTS file: DTS_demo.dtsi
 *
 * / {
 *        DTS_demo {
 *                compatible = "DTS_demo, BiscuitOS";
 *                status = "okay";
 *
 *                child0 {
 *                         compatible = "Child0, BiscuitOS";
 *                         status = "disabled";
 *                };
 *
 *                child1 {
 *                         compatbile = "Child1, BiscuitOS";
 *                         status = "okay";
 *                };
 *        };
 * };
 *
 * On Core dtsi:
 *
 * include "DTS_demo.dtsi"
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of_platform.h>
#include <linux/module.h>

/* define name for device and driver */
#define DEV_NAME "DTS_demo"

/* probe platform driver */
static int DTS_demo_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    struct device_node *child;
    int count;
  
    printk("DTS probe entence...\n");

    /* Count child number for current device node */
    count = of_get_child_count(np);
    printk("%s has %d children\n", np->name, count);

    printk("%s child:\n", np->name);
    for_each_child_of_node(np, child)
        printk("  \"%s\"\n", child->name);

    printk("%s available child:\n", np->name);
    for_each_available_child_of_node(np, child)
        printk("  \"%s\"\n", child->name);

    return 0;
}

/* remove platform driver */
static int DTS_demo_remove(struct platform_device *pdev)
{
    return 0;
}

static const struct of_device_id DTS_demo_of_match[] = {
    { .compatible = "DTS_demo, BiscuitOS",  },
    { },
};
MODULE_DEVICE_TABLE(of, DTS_demo_of_match);

/* platform driver information */
static struct platform_driver DTS_demo_driver = {
    .probe  = DTS_demo_probe,
    .remove = DTS_demo_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = DEV_NAME, /* Same as device name */
        .of_match_table = DTS_demo_of_match,
    },
};
module_platform_driver(DTS_demo_driver);
