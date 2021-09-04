/*
 * PCI Express Configure Register Reading by IO on BiscuitOS
 *
 * (C) 2021.09.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/io.h>

#define PCI_CFGADR		0xCF8
#define PCI_CFGDAT		0xCFC
/* Notice the BDF for BsicuitOS-pci: 00:04:00 */

static u32 BiscuitOS_pci_index(u8 bus, u8 device, u8 func, u8 reg)
{
	return (1 << 31) | ((bus & 0xff) << 16) | ((device & 0x1F) << 11) |
                           ((func & 0x7) << 8) | ((reg & 0x3F) << 2);
}

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	u8 bus, device, func, reg;
	u32 val, data;

	/* BDF for PCI device */
	bus    = 0;
	device = 4;
	func   = 0;
	reg    = 0x00; /* DeviceID:VendorID */

	/* Cover CFGADR Register value */
	val = BiscuitOS_pci_index(bus, device, func, reg);

	/* Obtain BDF PCI device Configure Register */
	outl(val, 0xCF8);
	data = inl(0xCFC);

	/* Display */
	printk("PCI %x:%x:%x Reg[%x]: %#x\n", bus, device, func, reg, data);

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("PCI Configureation IO Address on BiscuitOS");
