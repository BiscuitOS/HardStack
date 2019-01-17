/*
 * DTS: of_get_cpu_node
 *
 * struct device_node *of_get_cpu_node(int cpu, unsigned int *thread)
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
    const struct of_device_id *match;
    struct device_node *cpu_node;
    const char *comp = NULL;

    printk("DTS probe entence...\n");

    /* Read local CPU 0 */
    cpu_node = of_get_cpu_node(0, NULL);
    if (cpu_node) {
        printk("CPU0: %s\n", cpu_node->name);
        of_property_read_string(cpu_node, "compatible", &comp);
        if (comp)
            printk("  Compatible: %s\n", comp);
    }

    /* Read local CPU 1 */
    cpu_node = of_get_cpu_node(1, NULL);
    if (cpu_node) {
        printk("CPU1: %s\n", cpu_node->name);
        of_property_read_string(cpu_node, "compatible", &comp);
        if (comp)
            printk("  Compatible: %s\n", comp);
    }

    /* Read local CPU 2 */
    cpu_node = of_get_cpu_node(2, NULL);
    if (cpu_node) {
        printk("CPU2: %s\n", cpu_node->name);
        of_property_read_string(cpu_node, "compatible", &comp);
        if (comp)
            printk("  Compatible: %s\n", comp);
    }

    /* Read local CPU 3 */
    cpu_node = of_get_cpu_node(3, NULL);
    if (cpu_node) {
        printk("CPU3: %s\n", cpu_node->name);
        of_property_read_string(cpu_node, "compatible", &comp);
        if (comp)
            printk("  Compatible: %s\n", comp);
    }

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
