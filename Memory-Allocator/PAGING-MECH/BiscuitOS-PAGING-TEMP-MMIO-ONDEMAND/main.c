// SPDX-License-Identifier: GPL-2.0-only
/*
 * TEMPORARY MAPPING: MMIO OnDemand
 *
 * (C) 2023.11.03 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/cacheflush.h>
#include <linux/io.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-TEMP"
#define MMIO_BASE		0xF0000000UL
#define MMIO_SIZE		0x1000UL
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_ONDEMAND	_IO(BISCUITOS_IO, 0x00)

static struct resource Broiler_mmio_res = {
	.name	= "Broiler MMIO",
	.start	= MMIO_BASE,
	.end	= MMIO_BASE + MMIO_SIZE,
	.flags	= IORESOURCE_MEM,
};

static int ATPR_pte(pte_t *pte, unsigned long addr, void *data)
{
	struct vm_area_struct *vma = find_vma(current->mm, addr);
	void *src;

	if (!vma)
		return -EINVAL;

	/* TEMPORARY MAPPING */
	src = ioremap(MMIO_BASE, MMIO_SIZE);
	sprintf((char *)src, "Hello BiscuitOS");
	/* UC NON-FLUSH CACHE */
	/* TEMPORARY UNMAPPING */
	iounmap(src);

	/* BUILD PGTABLE */
	set_pte_at(vma->vm_mm, addr, pte, pte_mkspecial(
		   pfn_pte(MMIO_BASE >> PAGE_SHIFT, vma->vm_page_prot)));
	vma->vm_flags |= VM_PFNMAP | VM_MIXEDMAP;

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
	int r;

	r = request_resource(&iomem_resource, &Broiler_mmio_res);
	if (r < 0)
		return r;

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
