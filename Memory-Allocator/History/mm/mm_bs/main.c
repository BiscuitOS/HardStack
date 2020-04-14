/*
 * BiscuitOS (mm)
 *
 * (C) 2020.04.14 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * BiscuitOS RAM on DTS:
 *
 *       reserved-memory {
 *               #address-cells = <1>;
 *               #size-cells = <1>;
 *               ranges;
 *
 *               BiscuitOS_memory: memory@6a000000 {
 *                       reg = <0x6a000000 0x4000000>;
 *               };
 *
 *               vram: vram@4c000000 {
 *                       compatible = "shared-dma-pool";
 *                       reg = <0x4c000000 0x00800000>;
 *                       no-map;
 *               };
 *       };
 *
 *       BiscuitOS {
 *               compatible = "BiscuitOS,mm";
 *               status = "okay";
 *               ram = <&BiscuitOS_memory>;
 *       };
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>

#define DEV_NAME "BsicuitOS_memory"

/* RAM base address */
phys_addr_t BiscuitOS_ram_base;
phys_addr_t BiscuitOS_ram_size;

static int BiscuitOS_memory_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *mem;
	const phandle *ph;
	u32 array[2];
	int ret;

	/* Find memory node by phandle */
	ph = of_get_property(np, "ram", NULL);
	if (!ph) {
		printk("Unable to find 'phy-handle' on current device\n");
		return -EINVAL;
	}

	mem = of_find_node_by_phandle(be32_to_cpup(ph));
	if (!mem) {
		printk("Unable to find device node: BiscuitOS_memory\n");
		return -EINVAL;
	}

	/* Obtain memory information */
	ret = of_property_read_u32_array(mem, "reg", array, 2);
	if (ret) {
		printk("Unable to read BiscuitOS ram information\n");
		return -EINVAL;
	}
	BiscuitOS_ram_base = array[0];
	BiscuitOS_ram_size = array[1];

	/* Device Probe Procedure */
	printk("Hello BiscuitOS\n");

	return 0;
}

static int BiscuitOS_memory_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id BiscuitOS_memory_of_match[] = {
	{ .compatible = "BiscuitOS,memory", },
	{ },
};
MODULE_DEVICE_TABLE(of, BiscuitOS_memory_of_match);

/* Platform Driver Information */
static struct platform_driver BiscuitOS_memory_driver = {
	.probe    = BiscuitOS_memory_probe,
	.remove   = BiscuitOS_memory_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
		.of_match_table	= BiscuitOS_memory_of_match,
	},
};
module_platform_driver(BiscuitOS_memory_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS Memory Project");
