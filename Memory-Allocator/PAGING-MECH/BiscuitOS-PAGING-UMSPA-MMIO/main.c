// SPDX-License-Identifier: GPL-2.0
/*
 * UMSPA: MAPPING MMIO
 *
 * (C) 2023.11.03 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/io.h>

#define BROILER_MMIO_BASE	0xF0000000UL
#define BROILER_MMIO_LEN	0x1000UL

static struct resource Broiler_mmio_res = {
	.name	= "Broiler MMIO",
	.start	= BROILER_MMIO_BASE,
	.end	= BROILER_MMIO_BASE + BROILER_MMIO_LEN,
	.flags	= IORESOURCE_MEM,
};

static int __init BiscuitOS_init(void)
{
	return request_resource(&iomem_resource, &Broiler_mmio_res);
}

static void __exit BiscuitOS_exit(void)
{
	remove_resource(&Broiler_mmio_res);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS Paging Project");
