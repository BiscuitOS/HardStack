// SPDX-License-Identifier: GPL-2.0
/*
 * PFNMAP: PFNREMAP
 *
 *   - CMDLINE ADD "memmap=4K$0x10000000"
 *
 * (C) 2023.11.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-PFNMAP"
#define RSVDMEM_BASE		0x10000000
#define RSVDMEM_SIZE		0x1000

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return remap_pfn_range(vma, vma->vm_start,
			RSVDMEM_BASE >> PAGE_SHIFT,
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
MODULE_DESCRIPTION("BiscuitOS Paging Project");
