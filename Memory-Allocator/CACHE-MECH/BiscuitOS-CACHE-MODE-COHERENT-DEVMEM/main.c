/*
 * CACHE Mode Coherent with DEVMEM
 *
 * (C) 2023.02.14 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <asm/io.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-MEM"

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long pfn;
	struct page *page;

	/* Allocate Physical Memory */
	page = alloc_page(GFP_KERNEL);
	if (!page) {
		printk("System Error: No free Memory.\n");
		return -ENOMEM;
	}
	pfn = page_to_pfn(page);

	/* Clear PAT Attribute */
	pgprot_val(vma->vm_page_prot) &= ~(_PAGE_PCD | _PAGE_PWT | _PAGE_PAT);

	/* Change Memory Type for Direct-Mapping Area */
	arch_io_reserve_memtype_wc(PFN_PHYS(pfn), PAGE_SIZE);
	pgprot_val(vma->vm_page_prot) |= cachemode2protval(_PAGE_CACHE_MODE_WC);

	return remap_pfn_range(vma, vma->vm_start, pfn,
					PAGE_SIZE, vma->vm_page_prot);
}

/* file operations */
static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
};

static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= SPECIAL_DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

static int __init BiscuitOS_init(void)
{
	misc_register(&BiscuitOS_drv);

	return 0;
}

static void __exit BiscuitOS_exit(void)
{
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("DEVMEM with CACHE MODE");
