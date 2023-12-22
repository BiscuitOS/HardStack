// SPDX-License-Identifier: GPL-2.0-only
/*
 * MMAP: VM_OPS -> open
 *
 * (C) 2023.12.21 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/pfn_t.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-VMA-OPS"

static void vm_open(struct vm_area_struct *vma)
{
	printk("OPEN VMA %#lx - %#lx\n", vma->vm_start, vma->vm_end);
}

static const struct vm_operations_struct BiscuitOS_vm_ops = {
	.open	= vm_open,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct page *page = alloc_page(GFP_KERNEL);

	if (!page)
		return -ENOMEM;

	/* setup vm_ops */
	vma->vm_ops = &BiscuitOS_vm_ops;

	return remap_pfn_range(vma, vma->vm_start,
		  page_to_pfn(page), PAGE_SIZE, vma->vm_page_prot);
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
MODULE_DESCRIPTION("BiscuitOS MMU Project");
