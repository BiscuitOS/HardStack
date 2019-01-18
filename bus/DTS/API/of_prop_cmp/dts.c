/*
 * DTS: of_compat_cmp
 *      of_prop_cmp
 *      of_node_cmp
 *
 * #define of_compat_cmp(s1,s2,l) strcasecmp((s1)m(s2))
 * #define of_prop_cmp(s1,s2)     strcmp((s1), (s2))
 * #define of_node_cmp(s1,s2)     strcasecmp((s1),(s2))
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
    struct property *prop;
    const char *comp;

    printk("DTS probe entence...\n");

    /* Get property */
    prop = of_find_property(np, "compatible", NULL);

    /* Read comptatible proeprty */
    comp = of_get_property(np, "compatible", NULL);

    /* Compare compatible property value */
    if (of_compat_cmp(comp, "DTS_demo, BiscuitOS", strlen(comp)) == 0)
        printk("%s compatible: %s\n", np->name, comp);

    /* Compare proerpty name */
    if (of_prop_cmp(prop->name, "compatible") == 0)
        printk("%s property exist\n", prop->name);

    /* Compare node name */
    if (of_node_cmp(np->name, "DTS_demo") == 0)
        printk("%s node exist\n", np->name);

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
