// SPDX-License-Identifier: GPL-2.0
/*
 * OnDemand: IOCTL
 *
 * (C) 2023.11.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/highmem.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-ONDEMAND"
#define BISCUITOS_IO		0xBD
#define BISCUITOS_ONDEMAND	_IO(BISCUITOS_IO, 0x00)

static int handler_ondemand(struct mm_struct *mm, unsigned long addr)
{
	struct vm_area_struct *vma = find_vma(mm, addr);
	struct page *page = NULL;
	void *src;

	if (!vma)
		return -EINVAL;

	/* GUP */
	get_user_pages(addr, 1, FOLL_WRITE, &page, &vma);
	if (!page)
		return -ENOMEM;

	/* ONDEMAND */
	src = kmap(page);
	sprintf((char *)src, "Bello BiscuitOS");
	kunmap(page);

	return 0;
}

static long BiscuitOS_ioctl(struct file *filp,
				unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BISCUITOS_ONDEMAND:
		mmap_write_lock_killable(current->mm);
		handler_ondemand(current->mm, arg);
		mmap_write_unlock(current->mm);
		break;
	default:
		break;
	}

	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
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
