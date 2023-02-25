/*
 * DMABUF Import Kernel Stub
 *
 * (C) 2023.02.25 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/dma-buf.h>

/* DD Platform Name */
#define DEV_NAME		"BiscuitOS-DMABUF-import"
/* IOCTL CMD */
#define BISCUITOS_IO		0xBD
#define BISCUITOS_DMABUF_FD	_IO(BISCUITOS_IO, 0x00)

static struct dma_buf *BiscuitOS_dmabuf;

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	int fd, r;

	switch (ioctl) {
	case BISCUITOS_DMABUF_FD:
		r = copy_from_user(&fd, (void __user *)arg, sizeof(int));
		if (r < 0)
			return -EINVAL;

		BiscuitOS_dmabuf = dma_buf_get(fd);
		break;
	default:
		break;
	}
	return 0;
}

static int BiscuitOS_release(struct inode * inode, struct file *filp)
{
	dma_buf_put(BiscuitOS_dmabuf);
	return 0;
}

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return dma_buf_mmap(BiscuitOS_dmabuf, vma, vma->vm_pgoff);
}

/* file operations */
static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= BiscuitOS_ioctl,
	.release	= BiscuitOS_release,
	.mmap		= BiscuitOS_mmap,
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

MODULE_IMPORT_NS(DMA_BUF);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("DMABUF Import Stub");
