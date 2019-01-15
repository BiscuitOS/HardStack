/*
 * DTS: of_add_property
 *
 * int of_add_property(struct device_node *np, struct property *prop)
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

static u8  u8_data  = 0x10;
static u16 u16_data = 0x1020;
static u32 u32_data = 0x10203040;
static u64 u64_data = 0x1020304050607080;
static u32 u32_arr[]   = { 0x10203040, 0x50607080 };

static struct property BiscuitOS_u8 = {
    .name   = "BiscuitOS-u8",
    .length = sizeof(u8),
    .value  = &u8_data,
};

static struct property BiscuitOS_u16 = {
    .name   = "BiscuitOS-u16",
    .length = sizeof(u16),
    .value  = &u16_data,
};

static struct property BiscuitOS_u32 = {
    .name   = "BiscuitOS-u32",
    .length = sizeof(u32),
    .value  = &u32_data,
};

static struct property BiscuitOS_u64 = {
    .name   = "BiscuitOS-u64",
    .length = sizeof(u64),
    .value  = &u64_data,
};

static struct property BiscuitOS_u32arr = {
    .name   = "BiscuitOS-u32arry",
    .length = sizeof(u32) * 2,
    .value  = u32_arr,
};

static struct property BiscuitOS_string = {
    .name   = "BiscuitOS-string",
    .length = 10,
    .value  = "BiscuitOS",
};

/* probe platform driver */
static int DTS_demo_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    u8  u8_data;
    u16 u16_data;
    u32 u32_data;
    u64 u64_data;
    u32 u32_array[2];
    const char *string;
    const char *string_arr[2];
    int rc;

    printk("DTS demo probe entence.\n");

    /* Add a u8 data property into device node */
    of_add_property(np, &BiscuitOS_u8);
    /* Read it from DT */
    rc = of_property_read_u8(np, "BiscuitOS-u8", &u8_data);
    if (rc == 0)
        printk("BiscuitOS-u8:  %#x\n", u8_data);

    /* Add a u16 data property into device node */
    of_add_property(np, &BiscuitOS_u16);
    /* Read it from DT */
    rc = of_property_read_u16(np, "BiscuitOS-u16", &u16_data);
    if (rc == 0)
        printk("BiscuitOS-u16: %#x\n", be16_to_cpu(u16_data));

    /* Add a u32 data property into device node */
    of_add_property(np, &BiscuitOS_u32);
    /* Read it from DT */
    rc = of_property_read_u32(np, "BiscuitOS-u32", &u32_data);
    if (rc == 0)
        printk("BiscuitOS-u32: %#x\n", be32_to_cpu(u32_data));

    /* Add a u64 data property into device node */
    of_add_property(np, &BiscuitOS_u64);
    /* Read it from DT */
    rc = of_property_read_u64(np, "BiscuitOS-u64", &u64_data);
    if (rc == 0)
        printk("BiscuitOS-u64: %#llx\n", be64_to_cpu(u64_data));

    /* Add a u32 array property into device node */
    of_add_property(np, &BiscuitOS_u32arr);
    /* Read array from DT */
    rc = of_property_read_u32_array(np, "BiscuitOS-u32arry", u32_array, 2);
    if (rc == 0) {
        printk("BiscuitOS-u32arry[0]: %#x\n", be32_to_cpu(u32_array[0]));
        printk("BiscuitOS-u32arry[1]: %#x\n", be32_to_cpu(u32_array[1]));
    }

    /* Add a string property into device node */
    of_add_property(np, &BiscuitOS_string);
    /* Read string from DT */
    of_property_read_string(np, "BiscuitOS-string", &string);
    printk("BiscuitOS-string: %s\n", string);

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
