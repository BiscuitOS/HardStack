// SPDX-License-Identifier: GPL-2.0
/*
 * LOOKUP-ADDRESS: MMIO
 *
 * (C) 2023.11.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/io.h>

#define MMIO_BASE		0xF0000000UL
#define MMIO_SIZE		0x1000UL

static struct resource Broiler_mmio_res = {
	.name   = "Broiler MMIO",
	.start  = MMIO_BASE,
	.end    = MMIO_BASE + MMIO_SIZE,
	.flags  = IORESOURCE_MEM,
};

static int __init BiscuitOS_init(void)
{
	void *vaddr;
	pte_t *pte;
	int level;

	/* REGISTER MMIO RESOURCE */
	if (request_resource(&iomem_resource, &Broiler_mmio_res) < 0)
		return -EBUSY;
	
	/* REMAPPING MMIO */
	vaddr = ioremap(MMIO_BASE, MMIO_SIZE);
	if (!vaddr)
		return -ENOMEM;

	/* LOOKUP PTE */
	pte = lookup_address((unsigned long)vaddr, &level);
	printk("PADDR: %#lx\nVADDR: %#lx\nPTE:   %#lx LEVEL %d\n",
			MMIO_BASE, (unsigned long)vaddr,
			pte_val(*pte), level);

	/* RECLAIM */
	iounmap(vaddr);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
