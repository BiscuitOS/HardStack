// SPDX-License-Identifier: GPL-2.0
/*
 * PageTable Attribute
 *
 * (C) 2023.07.25 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-PageTable"

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct page *page = alloc_page(GFP_KERNEL);

	if (!page)
		return -ENOMEM;

	/* Setup Pgtable ReadOnly */
	vma->vm_page_prot = PAGE_READONLY;
	vma->vm_flags &= ~VM_WRITE;

	return remap_pfn_range(vma, vma->vm_start, page_to_pfn(page),
					  PAGE_SIZE, vma->vm_page_prot);
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
	return misc_register(&BiscuitOS_drv);
}

static void __exit BiscuitOS_exit(void)
{
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("PageTable Attribute");
