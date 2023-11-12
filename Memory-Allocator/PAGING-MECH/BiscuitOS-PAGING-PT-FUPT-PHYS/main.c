// SPDX-License-Identifier: GPL-2.0-only
/*
 * FOLLOW USERSPACE PGTABLE: Consult MMIO PHYS
 *
 * (C) 2023.11.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/mm.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-FUPT"
/* TRANS DATA */
struct bs_data {
	unsigned long vaddr;
	unsigned long phys;
	unsigned long prot;
} bsdata;

static ssize_t BiscuitOS_read(struct file *filp,
			char __user *buffer, size_t len, loff_t *off)
{
	struct vm_area_struct *vma;

	if (copy_from_user(&bsdata, buffer, sizeof(struct bs_data)))
		return -EINVAL;

	mmap_read_lock(current->mm);
	vma = find_vma(current->mm, bsdata.vaddr);
	if (!vma) {
		mmap_read_unlock(current->mm);
		return -EINVAL;
	}

	/* CONSULT PGTABLE */
	follow_phys(vma, bsdata.vaddr, 0, &bsdata.prot,
				(resource_size_t *)&bsdata.phys);
	mmap_read_unlock(current->mm);


	if (copy_to_user(buffer, &bsdata, sizeof(struct bs_data)))
		return -EINVAL;

	return len;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.read		= BiscuitOS_read,
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
__initcall(BiscuitOS_init);
