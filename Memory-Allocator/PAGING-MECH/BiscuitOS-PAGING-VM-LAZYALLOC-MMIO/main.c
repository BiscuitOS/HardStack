// SPDX-License-Identifier: GPL-2.0-only
/*
 * LAZYALLOC: MMIO Memory
 * 
 * (C) 2023.09.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-MMIO"
#define BROILER_MMIO_BASE	0xF0000000UL
#define BROILER_MMIO_LEN	0x1000UL

static int PFNMAP_pte(pte_t *pte, unsigned long addr, void *data)
{
	struct vm_area_struct *vma = (struct vm_area_struct *)data;

	set_pte_at(vma->vm_mm, addr, pte, pte_mkspecial(
		pfn_pte(BROILER_MMIO_BASE >> PAGE_SHIFT, vma->vm_page_prot)));

	return 0;
}

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;

	vma->vm_flags |= VM_PFNMAP | VM_MIXEDMAP;
	apply_to_page_range(vma->vm_mm, address,
			PAGE_SIZE, PFNMAP_pte, vma);

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

static struct resource Broiler_mmio_res = {
	.name	= "Broiler MMIO",
	.start	= BROILER_MMIO_BASE,
	.end	= BROILER_MMIO_BASE + BROILER_MMIO_LEN,
	.flags	= IORESOURCE_MEM,
};

static int __init BiscuitOS_init(void)
{
	if (request_resource(&iomem_resource, &Broiler_mmio_res) < 0)
		return -EBUSY;

	misc_register(&BiscuitOS_drv);

	return 0;
}

static void __exit BiscuitOS_exit(void)
{
	misc_deregister(&BiscuitOS_drv);
	remove_resource(&Broiler_mmio_res);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
