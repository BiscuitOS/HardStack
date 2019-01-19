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
 *        DTS_demo {
 *                compatible = "DTS_demo, BiscuitOS";
 *                status = "okay";
 *                DTS_demo_sub0 {
 *                        sub_level = <0x1>;
 *
 *                        DTS_demo_sub1 {
 *                                sub_level = <0x2>;
 *                        };
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
#include <linux/of_fdt.h>

/* define name for device and driver */
#define DEV_NAME "DTS_demo"

/* Parse specify device-tree structure node */
static int DTS_demo_dt_scan_node(unsigned long node, const char *uname,
                                        int depth, void *data)
{
    /* Filter father node */
    if ((depth == 1) && (strcmp(uname, "DTS_demo") == 0)) {
        printk("Father node: %s\n", uname);
    } else if ((depth == 2) && (strcmp(uname, "DTS_demo_sub0") == 0)) {
        /* Sub-level 1 Child node */
        printk("Sub-0 node: %s\n", uname);
    } else if ((depth == 3) && (strcmp(uname, "DTS_demo_sub1") == 0 )) {
        /* Sub-level 2 Child node */
        printk("Sub-1 node: %s\n", uname);
    }

    return 0;
}

/* probe platform driver */
static int DTS_demo_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    unsigned long dt_root;
    unsigned int *dt_int;
    const char *dt_char;
    
    printk("DTS demo probe entence\n");

    /* Rettrieve various infomation from the all node */
    of_scan_flat_dt(DTS_demo_dt_scan_node, NULL);

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
