/*
 * DTS: of_parse_phandle_with_args
 *
 * int of_parse_phandle_with_args(const struct device_node *np, 
 *                    const char *list_name, const char *cells_name,
 *                    int index, struct of_phandle_args *out_args)
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
 *                phy-handle = <&phy0 1 2 3 &phy1 4 5>;
 *                reset-gpio = <&gpio0 25 0>;
 *        };
 *
 *        phy0: phy@0 {
 *                #phy-cells = <3>;
 *                compatible = "PHY0, BiscuitOS";
 *        };
 *
 *        phy1: phy@1 {
 *                #phy-cells = <2>;
 *                compatible = "PHY1, BiscuitOS";
 *        };
 *
 *        gpio0: gpio@0 {
 *                #gpio-cells = <2>;
 *                compatible = "gpio, BiscuitOS";
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
    struct of_phandle_args args;
    int rc, index = 0;
    const u32 *comp;

    printk("DTS demo probe entence.\n");

    /* Read first phandle argument */
    rc = of_parse_phandle_with_args(np, "phy-handle", "#phy-cells",
                                        0, &args);
    if (rc < 0) {
        printk("Unable to parse phandle.\n");
        return -1;
    }
    
    comp = of_get_property(args.np, "compatible", NULL);
    if (comp)
        printk("%s compatible: %s\n", args.np->name, comp);

    for (index = 0; index < args.args_count; index++)
        printk("Args %d: %#x\n", index, args.args[index]);

    /* Read second phandle argument */
    rc = of_parse_phandle_with_args(np, "phy-handle", "#phy-cells",
                                        1, &args);
    if (rc < 0) {
        printk("Unable to parse phandle.\n");
        return -1;
    }
    
    comp = of_get_property(args.np, "compatible", NULL);
    if (comp)
        printk("%s compatible: %s\n", args.np->name, comp);

    for (index = 0; index < args.args_count; index++)
        printk("Args %d: %#x\n", index, args.args[index]);

    /* Read gpio phandle argument */
    rc = of_parse_phandle_with_args(np, "reset-gpio", "#gpio-cells",
                                 0, &args);
    if (rc < 0) {
        printk("Unable to parse gpio phandle.\n");
        return -1;
    }
    comp = of_get_property(args.np, "compatible", NULL);
    if (comp)
        printk("%s compatible: %s\n", args.np->name, comp);

    printk("GPIO index: %#x\n", args.args[0]);
    printk("GPIO egde:  %#x\n", args.args[1]);

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
