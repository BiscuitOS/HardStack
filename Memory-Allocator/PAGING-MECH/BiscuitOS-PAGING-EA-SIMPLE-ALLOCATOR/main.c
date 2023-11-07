// SPDX-License-Identifier: GPL-2.0
/*
 * PAGING Project: Simpler ALLOCATOR
 *
 *   CMDLINE ADD "memmap=256K$0x10000000" 
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/mmu_notifier.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-ALLOC"
#define MAX_PAGES		(64)
#define PFNPOOL_BASE		0x10000000
#define PFNPOOL_SIZE		(MAX_PAGES * PAGE_SIZE)
#define MAX_BITS		MAX_PAGES
#define MAX_BITS_LONG		((MAX_PAGES + (BITS_PER_LONG - 1)) / \
					       BITS_PER_LONG)

struct mmu_notifier notifier;
static long *bitmap;

/** PHYSCAL-MEMORY  ALLOCATOR**/

/* ALLOC PHYSICAL MEMORY */
static phys_addr_t alloc_phys(void)
{
	long index = find_first_zero_bit(bitmap, MAX_BITS);

	if (index >= 0) {
		bitmap_set(bitmap, index, index + 1); /* [index, index + 1) */
		return (phys_addr_t)(index * PAGE_SIZE + PFNPOOL_BASE);
	} else
		return -ENOMEM;
}

/* RECLAIM PHYSICAL MEMORY */
static void free_phys(phys_addr_t phys)
{
	long index = (phys - PFNPOOL_BASE) >> PAGE_SHIFT;

	bitmap_set(bitmap, index, index + 1); /* [index, index + 1) */
}

/* UN-MAPPING PGTABLE */
static int unmap_pte(pte_t *pte, unsigned long addr, void *data)
{
	if (pte_none(*pte))
		return 0;

	/* FREE */
	free_phys(pte_pfn(*pte) << PAGE_SHIFT);

	return 0;
}

/* MAPPING PGTABLE */
static int map_pte(pte_t *pte, unsigned long addr, void *data)
{
	struct vm_area_struct *vma = (struct vm_area_struct *)data;
	phys_addr_t phys;

	/* ALLOC PHYS MEM */
	phys = alloc_phys();
	if (phys < 0)
		return -ENOMEM;

	set_pte_at(vma->vm_mm, addr, pte,
		pte_mkspecial(pfn_pte(phys >> PAGE_SHIFT, vma->vm_page_prot)));

	return 0;
}

/* LISTEN FREE PGTABLE */
static int BiscuitOS_invalidate_range_start(struct mmu_notifier *mni,
			const struct mmu_notifier_range *range)
{
	if (range->event != MMU_NOTIFY_UNMAP)
		return 0;
	
	return apply_to_page_range(range->mm, range->start,
				PAGE_SIZE, unmap_pte, range->vma);
}

static const struct mmu_notifier_ops BiscuitOS_mn_ops = {
	.invalidate_range_start = BiscuitOS_invalidate_range_start,
};

/* PAGEFAULT */
static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;

	vma->vm_flags |= VM_PFNMAP | VM_MIXEDMAP;
	apply_to_page_range(vma->vm_mm, address,
				PAGE_SIZE, map_pte, vma);

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

static int BiscuitOS_open(struct inode *inode, struct file *filp)
{
	notifier.ops = &BiscuitOS_mn_ops; 
	mmu_notifier_register(&notifier, current->mm);

	return 0;
}

static int BiscuitOS_release(struct inode *inode, struct file *filp)
{
	mmu_notifier_unregister(&notifier, current->mm);

	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
	.open		= BiscuitOS_open,
	.release	= BiscuitOS_release,
};

static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= SPECIAL_DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

static int __init BiscuitOS_init(void)
{
	/* ALLOCATOR INIT */
	bitmap = kzalloc(sizeof(unsigned long) * MAX_BITS_LONG, GFP_KERNEL);
	if (!bitmap)
		return -ENOMEM;

	misc_register(&BiscuitOS_drv);

	return 0;
}

static void __exit BiscuitOS_exit(void)
{
	misc_deregister(&BiscuitOS_drv);
	kfree(bitmap);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
