/*
 * Device-Tree: of_get_address
 *
 * const __be32 *of_get_address(struct device_node *dev, int index, 
 *                    u64 *size, unsigned int *flags)
 *
 * (C) 2019.01.01 BuddyZhang1 <buddy.zhang@aliyun.com>
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
 *                reg = <0x11223344 0x55667788
 *                       0x10203040 0x50607080
 *                       0x99aabbcc 0xddeeff00
 *                       0x90a0b0c0 0xd0e0f000>;
 *        };
 * };
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>

/* define name for device and driver */
#define DEV_NAME "DTS_demo"

/* probe platform driver */
static int DTS_demo_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    const u32 *regaddr_p;
    u64 addr;
    
    printk("DTS demo probe entence.\n");

    /* get first address from 'reg' property */
    regaddr_p = of_get_address(np, 0, &addr, NULL);
    if (regaddr_p)
        printk("%s's address[0]: %#llx\n", np->name, addr);

    /* get second address from 'reg' property */
    regaddr_p = of_get_address(np, 1, &addr, NULL);
    if (regaddr_p)
        printk("%s's address[1]: %#llx\n", np->name, addr);

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
