// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault ERROR CODE: PF_USER Access KVADDR
 *
 * (C) 2023.09.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>

#define DEV_NAME		"BiscuitOS"
#define BISCUITOS_IO		0xAE
#define BISCUITOS_FAULT		_IO(BISCUITOS_IO, 0x00)

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	unsigned long addr;

	switch (ioctl) {
	case BISCUITOS_FAULT:
		addr = (unsigned long)vmalloc(PAGE_SIZE);
		if (!addr) {
			printk("ERROR: FAILE VMALLOC.\n");
			return -ENOMEM;
		}
		/* Write Ops, Don't Trigger #PF */
		*(char *)addr = 'B';
		/* Read Ops, Don't Trigger #PF */
		printk("VMALLOC %#lx => %c\n", addr, *(char *)addr);

		return addr;
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
__initcall(BiscuitOS_init);
