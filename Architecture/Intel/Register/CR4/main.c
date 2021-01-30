/*
 * Intel CR4 Register
 *
 *  CR4
 *
 *  31(63)                      22    19  17  15  13  11  9 8 7 6 5 4 3 2 1 0
 *  +--------------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          |P| | | | | | | | | | | | | |P|P|M|P|P| |T|P|V|
 *  |        Reserved          |K| | | | | | | | | | | | | |C|G|C|A|S|D|S|V|M|
 *  |                          |E| | | | | | | | | | | | | |E|E|E|E|E|E|D|I|E|
 *  +--------------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                                | |   | | |   | |   | | |
 *                                | |   | | |   | |   | | o------------------- OSFXSR
 *                                | |   | | |   | |   | o--------------------- OSXMMEXCPT
 *                                | |   | | |   | |   o----------------------- UMIP
 *                                | |   | | |   | o--------------------------- VMXE
 *                                | |   | | |   o----------------------------- SMXE
 *                                | |   | | o--------------------------------- FSGSBASE
 *                                | |   | o----------------------------------- PCIDE
 *                                | |   o------------------------------------- OSXSAVE
 *                                | o----------------------------------------- SMEP
 *                                o------------------------------------------- SMAP
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
/* Header */
#include <asm/special_insns.h>

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned long cr4;

	cr4 = __read_cr4();
	printk("CR4 Register %#lx\n", cr4);

	/* CR4.PAE */
	if (cr4 & X86_CR4_PAE)
		printk("The physical address more the 32 bits.\n");
	else
		printk("The physical address only 32 bits.\n");

	/* CR4.PSE */
	if (cr4 & X86_CR4_PSE)
		printk("System support mapping 4MB page.\n");
	else
		printk("System support mapping 4KB page.\n");

	/* CR4.PGE */
	if (cr4 & X86_CR4_PGE)
		printk("System support Global page.\n");
	else
		printk("Sustem doesn't support Global page.\n");

	/* CR4.PCIDE */
	if (cr4 & X86_CR4_PCIDE)
		printk("System support process-context identifiers (PCIDs).\n");
	else
		printk("System doesn't support process-context identifiers (PCIDs).\n");

	/* CR4.SMEP */
	if (cr4 & X86_CR4_SMEP)
		printk("System support supervisor-mode execution prevention.\n");
	else
		printk("System doesn't support supervisor-mode execution prevention.\n");

	/* CR4.SMEP */
	if (cr4 & X86_CR4_SMAP)
		printk("System support supevisor-mode access prevention.\n");
	else
		printk("System doesn't support supervisor-mode access prevention.\n");

	printk("Hello modules on BiscuitOS\n");

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Intel Register on BiscuitOS");
