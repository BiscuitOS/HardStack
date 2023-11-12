// SPDX-License-Identifier: GPL-2.0-only
/*
 * FOLLOW USERSPACE PGTABLE: Consult PTE
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
	unsigned long pte_val;
} bsdata;

static ssize_t BiscuitOS_read(struct file *filp,
			char __user *buffer, size_t len, loff_t *off)
{
	spinlock_t *lock;
	pte_t *pte;

	if (copy_from_user(&bsdata, buffer, sizeof(struct bs_data)))
		return -EINVAL;

	/* CONSULT PGTABLE */
	mmap_read_lock(current->mm);
	follow_pte(current->mm, bsdata.vaddr, &pte, &lock);
	pte_unmap_unlock(pte, lock);
	mmap_read_unlock(current->mm);

	/* TRANSLATION */
	bsdata.pte_val = pte_val(*pte);

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

static void __exit BiscuitOS_exit(void)
{
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
