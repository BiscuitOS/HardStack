/*
 * DTS: of_property_match_string
 *      of_property_count_strings
 *      of_property_read_string_index
 *      of_property_read_string_array
 *      of_property_read_string_helper
 *
 * int of_property_match_string(struct device_node *np, const char *propname,
 *                                      const char *string)
 *
 * static inline int of_property_count_strings(struct device_node *np,
 *                                      const char *propname)
 *
 * static inline int of_property_read_string_index(struct device_node *np,
 *                                      const char *propname,
 *                                      int index, const char **output)
 *
 * static inline int of_property_read_string_array(struct device_node *np,
 *                                      const char *propname, 
 *                                      const char **out_strs, size_t sz)
 *  
 * int of_property_read_string_helper(struct device_node *np, 
 *                                      const char *propname, 
 *                                      const char *out_strs, 
 *                                      size_t sz, int skip)
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
 *                BiscuitOS-strings = "uboot", "kernel", 
 *                                    "rootfs", "BiscuitOS";
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
    const char *string_array[4];
    const char *string;
    int count, index;
    int rc;

    printk("DTS demo probe entence.\n");

    /* Verify property whether contains special string. */
    rc = of_property_match_string(np, "BiscuitOS-strings", "rootfs");
    if (rc < 0) 
        printk("BiscuitOS-strings doesn't contain \"rootfs\"\n");
    else
        printk("BiscuitOS-strings[%d] is \"rootfs\"\n", rc);

    /* Count the string on string-list property */
    count = of_property_count_strings(np, "BiscuitOS-strings");
    printk("String count: %#x\n", count);

    /* Read special string on string-list property with index */
    for (index = 0; index < count; index++) {
        rc = of_property_read_string_index(np, "BiscuitOS-strings", 
                                                index, &string);
        if (rc < 0) {
            printk("Unable to read BiscuitOS-strings[%d]\n", index);
            continue;
        }
        printk("BiscuitOS-strings[%d]: %s\n", index, string);
    }
    
    /* Read number of strings from string-list property */
    rc = of_property_read_string_array(np, "BiscuitOS-strings", 
                                                string_array, count);
    if (rc < 0) {
        printk("Faild to invoke of_property_read_string_array()\n");
        return -1;
    }
    for (index = 0; index < count; index++)
        printk("String_array[%d]: %s\n", index, string_array[index]);

    /* Read a string with index on string-list property (index = 2) */
    rc = of_property_read_string_helper(np, "BiscuitOS-strings", 
                                                      &string, 1, 2);
    if (rc < 0)
        printk("Faild to invoke of_property_read_string_helper()!\n");
    else
        printk("BiscuitOS-strings[index=2]: %s\n", string);


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
