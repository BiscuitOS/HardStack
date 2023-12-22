// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: SPEICAL VIRTUAL MEMORY (LEGACY)
 *
 * (C) 2023.12.21 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/pagewalk.h>
#include <asm/pgtable.h>
#include <linux/io.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-SPECIAL"
#define BISCUITOS_IO		0xBD
#define BS_ALLOC		_IO(BISCUITOS_IO, 0x00)

static int special_mapping(unsigned long addr)
{
	struct mm_struct *mm = current->mm;
	struct page **pages, *page;
	int r;

	pages = kmalloc(sizeof(struct page *), GFP_KERNEL);
	if (!pages)
		return -ENOMEM;

	page = alloc_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM; /* IGNORE RECLAIM */

	/* BIND PHYSICAL ADDRESS*/
	*pages = page;

	if (mmap_write_lock_killable(mm))
		return -EINTR; /* IGNORE RECLAIM */

	/* CREATE SPECIAL MAPPING */
	r = install_special_mapping(mm, addr, PAGE_SIZE,
			VM_READ | VM_WRITE | VM_EXEC |
			VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC,
			pages);

	mmap_write_unlock(mm);

	return 0;
}

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BS_ALLOC:
		special_mapping(arg);
		break;
	}
	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= BiscuitOS_ioctl,
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
device_initcall(BiscuitOS_init);
