/*
 * DTS: of_get_parent
 *      of_get_next_parent
 *      of_get_next_child
 *      of_get_next_available_child
 *
 * struct device_node *of_get_parent(const struct device_node *node)
 *
 * struct device_node *of_get_next_parent(struct device_node *node)
 *
 * struct device_node *of_get_next_child(const struct device_node *node,
 *                                       struct device_node *prev)
 *
 * struct device_node *of_get_next_available_child(
 *                                       const struct device_node *node,
 *                                       struct device_node *prev)
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
 *                };
 *
 *                child1 {
 *                         compatbile = "Child1, BiscuitOS";
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
    struct device_node *parent;
    struct device_node *nparent;
    struct device_node *child;
    struct device_node *nchild;
  
    printk("DTS probe entence...\n");

    /* Find child */
    child = of_get_next_child(np, NULL);
    if (child)
        printk("\"%s\"'s child: \"%s\"\n", np->full_name, child->full_name);
    else
        printk("The device node doesn't have child\n");

    /* Find next child */
    nchild = of_get_next_available_child(np, child);
    if (nchild)
        printk("\"%s\"'s next child: \"%s\"\n", np->full_name, 
                                                nchild->full_name);

    /* find parent */
    parent = of_get_parent(child);
    if (parent) 
        printk("\"%s\"'s parent: \"%s\"\n", child->full_name, 
                                            parent->full_name);

    /* Lterate to a node's parent */
    nparent = of_get_next_parent(child);
    if (nparent)
        printk("\"%s\"'s next parent: \"%s\"\n", child->full_name, 
                                                 parent->full_name);

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
