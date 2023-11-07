// SPDX-License-Identifier: GPL-2.0-only
/*
 * ATPR(Apply to page range): Consult PageTable
 *
 * (C) 2023.10.26 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-ATPR"
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_CONSULT	_IO(BISCUITOS_IO, 0x00)

static int ATPR_pte(pte_t *pte, unsigned long addr, void *data)
{
	printk("ATPR: %#lx PTE %#lx\n", addr, pte_val(*pte));
	return 0;
}

static long BiscuitOS_ioctl(struct file *filp,
				unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BISCUITOS_CONSULT:
		apply_to_existing_page_range(current->mm, arg,
					PAGE_SIZE, ATPR_pte, NULL);
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
MODULE_DESCRIPTION("PAGING");
