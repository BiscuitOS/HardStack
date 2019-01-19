/*
 * DTS: 
 *    of_get_flat_dt_root()
 *    of_get_flat_dt_prop()
 *
 * (C) 2018.11.14 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 * Private DTS file: DTS_demo.dtsi
 *
 * / {
 *        BiscuitOS-int = <0x10203040>;
 *        BiscuitOS-name = "BiscuitOS";
 * 
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
#include <linux/of_fdt.h>

/* define name for device and driver */
#define DEV_NAME "DTS_demo"

/* probe platform driver */
static int DTS_demo_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    unsigned long dt_root;
    const __be32 *dt_int;
    const char *dt_char;
    u64 value;
    int len;
    
    printk("DTS demo probe entence\n");

    /* find the root node in the flat blob */
    dt_root = of_get_flat_dt_root();

    /* Obtain int property */
    dt_int = of_get_flat_dt_prop(dt_root, "BiscuitOS-int", &len);
    /* Obtain char * property */
    dt_char = of_get_flat_dt_prop(dt_root, "BiscuitOS-name", NULL);

    if (dt_int > 0) {
        value = of_read_number(dt_int, len / 4);
        printk("BiscuitOS-int property: %#llx\n", (unsigned long long)value);
    }

    if (dt_char)
        printk("BiscuitOS-name property: %s\n", dt_char);

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
