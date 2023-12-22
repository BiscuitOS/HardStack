// SPDX-License-Identifier: GPL-2.0-only
/*
 * MMAP: VM_OPS -> fault
 * 
 * (C) 2023.12.21 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-VMA-OPS"

static int pte_handler(pte_t *pte, unsigned long addr, void *data)
{
	struct vm_area_struct *vma = find_vma(current->mm, addr);
	struct page *page = (struct page *)data;

	if (!vma)
		return -EINVAL;

	set_pte_at(vma->vm_mm, addr, pte,
		pte_mkspecial(pfn_pte(page_to_pfn(page), vma->vm_page_prot)));

	return 0;
}

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;
	struct page *page = alloc_page(GFP_KERNEL);

	if (!page)
		return VM_FAULT_OOM; 

	vma->vm_flags |= VM_PFNMAP | VM_MIXEDMAP;
	apply_to_page_range(vma->vm_mm, address,
				PAGE_SIZE, pte_handler, page);

	return VM_FAULT_NOPAGE;
}

static const struct vm_operations_struct BiscuitOS_vm_ops = {
	.fault	= vm_fault,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	/* setup vm_ops */
	vma->vm_ops = &BiscuitOS_vm_ops;

	return 0;
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
