// SPDX-License-Identifier: GPL-2.0-only
/*
 * PageFault with VMALLOC32
 *
 * (C) 2023.09.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-VMALLOC"

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	void *mem;

	mem = vmalloc_32_user(PAGE_SIZE);
	if (!mem) {
		printk("System Error: No free VMALLOC memory!\n");
		return -EAGAIN;
	}

	/* VMA must contain VM_MIXEDMAP on Page-Fault route */
	vma->vm_flags |= VM_MIXEDMAP;
	/* Establish User Pagetable */
	remap_vmalloc_range(vma, mem, 0);

	/* bind fault page */
	vmf->page = vmalloc_to_page(mem);

	return 0;
}

static const struct vm_operations_struct BiscuitOS_vm_ops = {
	.fault	= vm_fault,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	/* setup vm_ops */
	vma->vm_ops = &BiscuitOS_vm_ops;

	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
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
MODULE_DESCRIPTION("PageFault");
