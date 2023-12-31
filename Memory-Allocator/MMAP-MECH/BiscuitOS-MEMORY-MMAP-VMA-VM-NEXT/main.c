// SPDX-License-Identifier: GPL-2.0
/*
 * VMA: VMA VM_NEXT/VM_PREV
 *
 * (C) 2023.12.25 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

#define DEV_NAME		"BiscuitOS-VMA"
#define BISCUITOS_IO		0xBD
#define TRAVER_VMA		_IO(BISCUITOS_IO, 0x00)

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	struct vm_area_struct *vma;
	struct mm_struct *mm;

	switch (ioctl) {
	case TRAVER_VMA:
		/* Process Virtual Address Space */
		mm = current->mm;

		if (!mm->mmap) {
			printk("Process Virtual Address Space Empty.\n");
			return -EINVAL;
		}

		/* Traver ALL VMA */
		for (vma = mm->mmap; vma; vma = vma->vm_next)
			printk("VMA: %#lx - %#lx\n",
						vma->vm_start, vma->vm_end);

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
MODULE_DESCRIPTION("BiscuitOS MMU");
