// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk with IOREMAP MMIO
 *
 * (C) 2023.08.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/pagewalk.h>
#include <linux/io.h>

#define BROILER_MMIO_BASE       0xF0000000UL
#define BROILER_MMIO_LEN        0x1000UL

static struct resource Broiler_mmio_res = {
	.name   = "Broiler MMIO",
	.start  = BROILER_MMIO_BASE,
	.end    = BROILER_MMIO_BASE + BROILER_MMIO_LEN,
	.flags  = IORESOURCE_MEM,
};
static void __iomem *mmio;

static int BiscuitOS_pte_entry(pte_t *pte, unsigned long addr,
			unsigned long next, struct mm_walk *walk)
{
	if (pte_none(*pte))
		return 0;

	printk("Virtual Addr: %#lx\n", addr);
	printk("PageTB PMD:   %#lx\n", pte_val(*pte));
	printk("MMIO:         %#lx\n", pte_pfn(*pte) << PAGE_SHIFT);

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pte_entry	= BiscuitOS_pte_entry,
};

static int __init BiscuitOS_init(void)
{
	int r = request_resource(&iomem_resource, &Broiler_mmio_res);

	if (r < 0)
		return r;

	mmio = ioremap(BROILER_MMIO_BASE, BROILER_MMIO_LEN);
	if (!mmio) {
		printk("IOREMAP MMIO Failed.\n");
		remove_resource(&Broiler_mmio_res);
		return -ENOMEM;
	}
	printk("IOREMAP VA: %#lx PFN: %#lx\n", (unsigned long)mmio, 
					BROILER_MMIO_BASE);

	mmap_write_lock_killable(&init_mm);

	walk_page_range_novma(&init_mm, (unsigned long)mmio,
			(unsigned long)mmio + PAGE_SIZE,
			&BiscuitOS_pwalk_ops, init_mm.pgd, NULL);

	mmap_write_unlock(&init_mm);

	iounmap(mmio);

	return 0;
}
device_initcall(BiscuitOS_init);
