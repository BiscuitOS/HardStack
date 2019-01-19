/*
 * DTS: of_get_property
 *
 * const void *of_get_property(const struct device_node *np, const char *name)
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
 *                BiscuitOS_name = "BiscuitOS";
 *                BiscuitOS_int  = <0x10203040>;
 *                BiscuitOS_mult = <0x10203040 0x50607080 
                                    0x90a0b0c0 0xd0e0f000>;
 *                BiscuitOS_leg;
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

/* define name for device and driver */
#define DEV_NAME "DTS_demo"

/* probe platform driver */
static int DTS_demo_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    const __be32 *prop;
    int len;
    
    printk("DTS demo probe entence\n");

    /* Obtain string property from device node */
    prop = of_get_property(np, "BiscuitOS_name", &len);
    if (prop) {
        printk("Property: BiscuitOS_name\n");
        printk("String: %s\n", prop);
    }

    /* Obtain int property from device node */
    prop = of_get_property(np, "BiscuitOS_int", &len);
    if (prop) {
        int value;

        value = be32_to_cpup(prop);
        printk("Property: BiscuitOS_int\n");
        printk("Value: %#x\n", value);
    }

    /* Obtain multi-int property from device node */
    prop = of_get_property(np, "BiscuitOS_mult", NULL);
    if (prop) {
        unsigned long value;

        printk("Property: BiscuitOS_mult\n");
        /* #cells 0 value */
        value = of_read_ulong(prop, of_n_addr_cells(np));
        printk("#cell 0: %#llx\n", value);

        /* #cells 1 value */
        prop += of_n_addr_cells(np);
        value = of_read_ulong(prop, of_n_addr_cells(np));
        printk("#cell 1: %#llx\n", value);
    }

    /* Obtain empty-value property on device node */
    prop = of_get_property(np, "BiscuitOS_leg", NULL);
    if (prop)
        printk("BiscuitOS_leg property exist.\n");
    else
        printk("BiscuitOS_leg property doesn't exist.\n");
   
  
    return 0;
}

/* remove platform driver */
static int DTS_demo_remove(struct platform_device *pdev)
{
    return 0;
}

static const struct of_device_id DTS_demo_of_match[] = {
    { .compatible = "DTS_demo, BiscuitOS", },
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
