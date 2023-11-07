// SPDX-License-Identifier: GPL-2.0-only
/*
 * PFNMAP: OnDemand
 *
 *  - CMDLINE ADD 'memmap=4K$0x10000000'
 *
 * (C) 2023.11.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/io.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-PFNMAP"
#define RSVDMEM_BASE		0x10000000
#define RSVDMEM_SIZE		0x1000
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_ONDEMAND	_IO(BISCUITOS_IO, 0x00)

static int PFNMAP_pte(pte_t *pte, unsigned long addr, void *data)
{
	struct vm_area_struct *vma = find_vma(current->mm, addr);
	void *src;

	if (!vma)
		return -EINVAL;

	/* OnDemand */
	src = memremap(RSVDMEM_BASE, RSVDMEM_SIZE, MEMREMAP_WB);
	sprintf((char *)src, "Hello BiscuitOS");
	memunmap(src);

	set_pte_at(vma->vm_mm, addr, pte, pte_mkspecial(
		pfn_pte(RSVDMEM_BASE >> PAGE_SHIFT, vma->vm_page_prot)));

	return 0;
}

static long BiscuitOS_ioctl(struct file *filp,
				unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BISCUITOS_ONDEMAND:
		mmap_write_lock_killable(current->mm);
		apply_to_page_range(current->mm, arg,
					PAGE_SIZE, PFNMAP_pte, NULL);
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
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
