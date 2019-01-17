/*
 * DTS: of_n_addr_cells
 *      of_n_size_cells
 *
 * int of_n_addr_cells(struct device_node *np)
 * 
 * int of_n_size_cells(struct device_node *np)
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

/* define name for device and driver */
#define DEV_NAME "DTS_demo"

/* probe platform driver */
static int DTS_demo_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    int cells;

    printk("DTS probe entence...\n");

    /* Get #address-cells number */
    cells = of_n_addr_cells(np);
    printk("%s #address-cells: %#x\n", np->name, cells);

    /* Get #size-cells number */
    cells = of_n_size_cells(np);
    printk("%s #size-cells:    %#x\n", np->name, cells);

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
