/*
 * RSVDMEM with variable Memory Type
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
#define RSVD_MEM_BASE		0x10000000
#define RSVD_MEM_SIZE		0x200000

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	enum page_cache_mode pcm = vma->vm_pgoff; /* PAT from pgoff */

	/* Clear PCD/PAT/PWT */
	pgprot_val(vma->vm_page_prot) &= ~(_PAGE_PCD | _PAGE_PWT | _PAGE_PAT);

	/* _PAGE_PAT: _PAGE_PCD: _PAGE_PWT */
	pgprot_val(vma->vm_page_prot) |= cachemode2protval(pcm);

	return remap_pfn_range(vma, vma->vm_start,
		(RSVD_MEM_BASE + vma->vm_pgoff) >> PAGE_SHIFT,
		vma->vm_end - vma->vm_start, vma->vm_page_prot);
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
MODULE_DESCRIPTION("RSVDMEM with CACHE MODE");
