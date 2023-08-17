// SPDX-License-Identifier: GPL-2.0
/*
 * PTDUMP: NX Check
 *
 * (C) 2023.08.12 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/set_memory.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-PTDUMP"

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct page *page = alloc_page(GFP_KERNEL);
	unsigned long addr;

	if (!page) {
		printk("System Error: No free Memory.\n");
		return -ENOMEM;
	}
	addr = (unsigned long)page_address(page);
	printk("Kernel Address %#lx\n", addr);
	
	/* Mark W+X */
	set_memory_rw(addr, 1);
	set_memory_x(addr, 1);
	printk("Check W+X\n");
	debug_checkwx();

	/* Mark W+NX */
	set_memory_nx(addr, 1);
	printk("Check W+NX\n");
	debug_checkwx();
	
	return remap_pfn_range(vma, vma->vm_start, page_to_pfn(page),
				PAGE_SIZE, vma->vm_page_prot);
}

/* file operations */
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
__initcall(BiscuitOS_init);
