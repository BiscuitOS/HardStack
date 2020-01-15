/*
 * DTS: of_address_to_resource
 *
 * (C) 2020.01.15 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Private DTS file: DTS_demo.dtsi
 *
 * / {
 *        DTS@58000000 {
 *                compatible = "DTS, BiscuitOS";
 *                reg = <0x58000000 0x100
 *                       0x58100000 0x100
 *                       0x58200000 0x100>;
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
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>

/* DDL Platform Name */
#define DEV_NAME		"DTS"
#define RESOURCE_REG_NUM	3

static void __iomem *base[RESOURCE_REG_NUM];

/* Probe: (DDL) Initialize Device */
static int DTS_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct resource iomem[RESOURCE_REG_NUM];
	int err, idx;

	for (idx = 0; idx < RESOURCE_REG_NUM; idx++) {
		err = of_address_to_resource(np, idx, &iomem[idx]);
		if (err) {
			dev_err(dev, "%d could not get IO memory\n", idx);
			return err;
		}

		base[idx] = devm_ioremap_resource(dev, &iomem[idx]);
		if (IS_ERR(base[idx])) {
			dev_err(dev, "%d could not IO remap\n", idx);
			return PTR_ERR(base[idx]);
		}
		printk("BiscuitOS: %#lx - %#lx\n", 
				(unsigned long)iomem[idx].start, 
				(unsigned long)iomem[idx].end);
	}

	return 0;
}

/* Remove: (DDL) Remove Device (Module) */
static int DTS_remove(struct platform_device *pdev)
{
	int idx;

	for (idx = 0; idx < RESOURCE_REG_NUM; idx++)
		devm_iounmap(&pdev->dev, base[idx]);
	printk("Remove BiscuitOS.\n");
	return 0;
}

static const struct of_device_id DTS_of_match[] = {
	{ .compatible = "DTS, BiscuitOS", },
	{ },
};
MODULE_DEVICE_TABLE(of, DTS_of_match);

/* Platform Driver Information */
static struct platform_driver DTS_driver = {
	.probe    = DTS_probe,
	.remove   = DTS_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
		.of_match_table	= DTS_of_match,
	},
};
module_platform_driver(DTS_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("DTS device driver");
