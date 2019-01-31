/*
 * DTS: of_remove_property
 *
 * int of_remove_property(struct device_node *np, struct property *prop)
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
 *                BiscuitOS-u32 = <0x50607080>;
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

static u32 u32_data = 0x10203040;

static struct property BiscuitOS_u32 = {
    .name   = "BiscuitOS-u32",
    .length = sizeof(u32),
    .value  = &u32_data,
};

/* probe platform driver */
static int DTS_demo_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    struct property *prop;
    u32 u32_data;
    int rc;

    printk("DTS demo probe entence.\n");

    /* Read legacy property from DT */
    rc = of_property_read_u32(np, "BiscuitOS-u32", &u32_data);
    if (rc == 0)
        printk("Legacy BiscuitOS-u32: %#x\n", be32_to_cpu(u32_data));

    /* Remove speical property from DT */
    prop = of_find_property(np, "BiscuitOS-u32", NULL);
    if (prop)
        of_remove_property(np, prop);

    /* Add a u32 data property into device node */
    of_add_property(np, &BiscuitOS_u32);
    /* Read it from DT */
    rc = of_property_read_u32(np, "BiscuitOS-u32", &u32_data);
    if (rc == 0)
        printk("New BiscuitOS-u32:    %#x\n", be32_to_cpu(u32_data));

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
