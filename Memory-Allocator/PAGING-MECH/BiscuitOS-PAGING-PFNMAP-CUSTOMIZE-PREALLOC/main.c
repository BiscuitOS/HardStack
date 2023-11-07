// SPDX-License-Identifier: GPL-2.0
/*
 * PFNMAP: CUSTOMIZE PreALLOC
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

static int PFNMAP_pte(pte_t *pte, unsigned long addr, void *data)
{
	struct vm_area_struct *vma = (struct vm_area_struct *)data;

	vma->vm_flags |= VM_IO | VM_PFNMAP;
	set_pte_at(vma->vm_mm, addr, pte, pte_mkspecial(
		pfn_pte(RSVDMEM_BASE >> PAGE_SHIFT, vma->vm_page_prot)));

	return 0;
}

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return apply_to_page_range(vma->vm_mm, vma->vm_start,
				RSVDMEM_SIZE, PFNMAP_pte, (void *)vma);
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
