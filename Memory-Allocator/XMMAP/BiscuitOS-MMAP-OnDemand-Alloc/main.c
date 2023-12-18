// SPDX-License-Identifier: GPL-2.0
/*
 * On-Deamnd Alloc Memory
 *
 * (C) 2023.04.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/highmem.h>

#define DEV_NAME		"BiscuitOS"
#define BISCUITOS_IO		0xAE
#define BISCUITOS_ONDEMAND	_IO(BISCUITOS_IO, 0x00)

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	struct page *page;
	void *addr;
	int r;

	switch (ioctl) {
	case BISCUITOS_ONDEMAND:
		/* GUP: Alloc page On-Demand */
		r = get_user_pages(arg, 1, 0, &page, NULL);
		if (r < 1)
			return -EFAULT;
		/* Temp mapping */
		addr = kmap_atomic(page);
		sprintf((char *)addr, "Hello BiscuitOS");
		kunmap_atomic(addr);
		put_page(page);
		break;
	default:
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
	.name	= DEV_NAME,
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
MODULE_DESCRIPTION("OnDemand Alloc");
