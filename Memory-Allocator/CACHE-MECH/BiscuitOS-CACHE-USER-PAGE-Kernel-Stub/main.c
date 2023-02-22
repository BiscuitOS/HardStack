/*
 * Memory Type Kernel Stub for Userspace
 *  - Must add 'memmap=2M$0x10000000' into cmdline
 *
 * (C) 2023.02.08 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <linux/mm.h>
#include <asm/pgtable_types.h>
#include <asm/mtrr.h>

#define DEV_NAME			"BiscuitOS-CACHE-MEM"
#define MTRR_MEM_BASE			0x10000000
#define MTRR_MEM_SIZE			0x200000
/* Memory Type and Mnemonic */
#define MTRR_MEMTYPE_UC			0x00
#define MTRR_MEMTYPE_WC			0x01
#define MTRR_MEMTYPE_WT			0x04
#define MTRR_MEMTYPE_WP			0x05
#define MTRR_MEMTYPE_WB			0x06

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	enum page_cache_mode pcm = vma->vm_pgoff; /* PAT from pgoff */

	/* Clear PCD/PAT/PWT */
	pgprot_val(vma->vm_page_prot) &= ~(_PAGE_PCD | _PAGE_PWT | _PAGE_PAT);

	/* _PAGE_PAT: _PAGE_PCD: _PAGE_PWT */
	pgprot_val(vma->vm_page_prot) |= cachemode2protval(pcm);

	return remap_pfn_range(vma, vma->vm_start,
			(MTRR_MEM_BASE + vma->vm_pgoff) >> PAGE_SHIFT,
			vma->vm_end - vma->vm_start, vma->vm_page_prot);
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
};

static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

static int __init BiscuitOS_init(void)
{
	/* MTRR */
	mtrr_add(MTRR_MEM_BASE, MTRR_MEM_SIZE, MTRR_MEMTYPE_WB, true);
	misc_register(&BiscuitOS_drv);
	return 0;
}
device_initcall(BiscuitOS_init);
