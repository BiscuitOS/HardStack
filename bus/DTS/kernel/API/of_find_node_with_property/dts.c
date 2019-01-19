/*
 * DTS: of_find_node_with_property
 *
 * struct device_node *of_find_node_with_property(struct device_node *from,
 *                                    const char *prop_name)
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
 *                BiscuitOs-name = "BiscuitOS0";
 *                status = "okay";
 *        };
 *        DTS_demoX {
 *                compatible = "DTS_demo, BiscuitOSX";
 *                BiscuitOs-name = "BiscuitOS1";
 *                status = "disabled";
 *        };
 *        DTS_demoY {
 *                compatible = "DTS_demo, BiscuitOSY";
 *                BiscuitOs-name = "BiscuitOS2";
 *                status = "disabled";
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
    struct device_node *node;

    printk("DTS probe entence...\n");

    /* find device node via property name */
    node = of_find_node_with_property(NULL, "BiscuitOs-name");
    if (node)
        printk("Found %s\n", node->full_name); 

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
