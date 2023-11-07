// SPDX-License-Identifier: GPL-2.0-only
/*
 * ATPR(Apply to page range): OnDemand
 *
 * (C) 2023.10.26 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/highmem.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-ATPR"
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_ONDEMAND	_IO(BISCUITOS_IO, 0x00)

static int ATPR_pte(pte_t *pte, unsigned long addr, void *data)
{
	struct vm_area_struct *vma = find_vma(current->mm, addr);
	struct page *page;
	void *src;

	if (!vma)
		return -EINVAL;

	page = alloc_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;

	/* OnDemand */
	src = kmap(page);
	sprintf((char *)src, "Hello BiscuitOS");
	kunmap(page);

	set_pte_at(vma->vm_mm, addr, pte,
		pte_mkspecial(pfn_pte(page_to_pfn(page), vma->vm_page_prot)));

	return 0;
}

static long BiscuitOS_ioctl(struct file *filp,
				unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BISCUITOS_ONDEMAND:
		mmap_write_lock_killable(current->mm);
		apply_to_page_range(current->mm, arg,
					PAGE_SIZE, ATPR_pte, NULL);
		mmap_write_unlock(current->mm);
		break;
	default:
		break;
	}

	return 0;
}

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	vma->vm_flags |= VM_PFNMAP | VM_MIXEDMAP;
	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
	.unlocked_ioctl = BiscuitOS_ioctl,
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
MODULE_DESCRIPTION("PAGING");
