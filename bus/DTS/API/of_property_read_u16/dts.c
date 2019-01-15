/*
 * DTS: of_property_read_bool
 *      of_property_read_u8
 *      of_property_read_u16
 *      of_property_read_u32
 *
 * static inline bool of_property_read_bool(const struct device_node *np,
 *                            const char *propname)
 *  
 * static inline int of_property_read_u8(const struct device_node *np,
 *                            const char *propname,
 *                            u8 *out_value)
 *  
 * static inline int of_property_read_u16(const struct device_node *np,
 *                            const char *propname,
 *                            u16 *out_value)
 *  
 * static inline int of_property_read_u32(const struct device_node *np,
 *                            const char *propname,
 *                            u32 *out_value)
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
 *                BiscuitOS-data = <0x10203040 0x50607080>;
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
    bool bool_data;
    u8 u8_data;
    u16 u16_data;
    u32 u32_data;

    printk("DTS demo probe entence.\n");

    /* Read bool data from property */
    bool_data = of_property_read_bool(np, "BiscuitOS-data");
    printk("bool_data: %#x\n", bool_data);

    /* Read u8 data from property */
    of_property_read_u8(np, "BiscuitOS-data", &u8_data);
    printk("u8_data:   %#x\n", u8_data);

    /* Read u16 data from property */
    of_property_read_u16(np, "BiscuitOS-data", &u16_data);
    printk("u16_data:  %#x\n", u16_data);

    /* Read u32 data from property */
    of_property_read_u32(np, "BiscuitOS-data", &u32_data);
    printk("u32_data:  %#x\n", u32_data);

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
