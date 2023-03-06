/*
 * Mapping WT MMIO into Userspace
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
#include <linux/fs.h>
#include <linux/mm.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-MMIO"
#define BROILER_MMIO_BASE	0xF0000000
#define BROILER_MMIO_LEN	0x1000

static struct resource Broiler_mmio_res = {
	.name   = "Broiler MMIO",
	.start  = BROILER_MMIO_BASE,
	.end    = BROILER_MMIO_BASE + BROILER_MMIO_LEN,
	.flags  = IORESOURCE_MEM,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	/* Clear PCD/PAT/PWT */
	pgprot_val(vma->vm_page_prot) &= ~(_PAGE_PCD | _PAGE_PWT | _PAGE_PAT);
	/* Setup WT */
	vma->vm_page_prot = pgprot_writethrough(vma->vm_page_prot);

	return io_remap_pfn_range(vma, vma->vm_start,
			BROILER_MMIO_BASE >> PAGE_SHIFT,
			vma->vm_end - vma->vm_start, vma->vm_page_prot);
}

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
	int r;

	r = request_resource(&iomem_resource, &Broiler_mmio_res);
	if (r < 0)
		return r;

	misc_register(&BiscuitOS_drv);

	return 0;
}

static void __exit BiscuitOS_exit(void)
{
	misc_deregister(&BiscuitOS_drv);
	release_resource(&iomem_resource);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("WC MMIO on Userspace");
