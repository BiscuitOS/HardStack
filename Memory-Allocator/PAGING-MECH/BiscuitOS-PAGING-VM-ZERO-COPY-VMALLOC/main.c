// SPDX-License-Identifier: GPL-2.0
/*
 * ZERO COPY: VMALLOC
 *
 * (C) 2023.11.21 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-ZERO"
#define ZC_IO			0xBD
#define ZC_COPY_FROM_USER	_IO(ZC_IO, 0x00)

static void *mem;

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	mem = vmalloc_user(PAGE_SIZE);
	if (!mem) {
		printk("System Error: No free VMALLOC Memory.\n");
		return -ENOMEM;
	}

	return remap_vmalloc_range(vma, mem, 0);
}

static long BiscuitOS_ioctl(struct file *filp, unsigned int cmd,
							unsigned long arg)
{
	switch (cmd) {
	case ZC_COPY_FROM_USER:
		/* ZERO COPY FROM USER */
		printk("ZERO-COPY VMALLOC: %s\n", (char *)mem);
		break;
	}

	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
	.unlocked_ioctl	= BiscuitOS_ioctl,
};

static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= SPECIAL_DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

static int __init BiscuitOS_init(void)
{
	vfree(mem);
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
