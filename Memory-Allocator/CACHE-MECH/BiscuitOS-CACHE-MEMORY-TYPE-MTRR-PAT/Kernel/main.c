/*
 * Set Memory Type for Userspace: MTRR and PAT
 *   - Add 'memmap=2M$0x10000000' into cmdline.
 *
 * (C) 2020.10.06 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <linux/mm.h>
#include <uapi/asm/mtrr.h>
#include <linux/mm_types.h>
#include <asm/mtrr.h>
#include <asm/pgtable_types.h>

#define DEV_NAME		"BiscuitOS-CACHE"
/* Memory Range */
#define MTRR_MEM_BASE		0x10000000
#define MTRR_MEM_SIZE		0x200000
/* Memory Type and Mnemonic */
#define MTRR_MEMTYPE_UC		0x00
#define MTRR_MEMTYPE_WC		0x01
#define MTRR_MEMTYPE_WT		0x04
#define MTRR_MEMTYPE_WP		0x05
#define MTRR_MEMTYPE_WB		0x06
/*
 * Setup Effective Page-Level Memory Types
 * +------------------+-----------------+-----------------------+
 * | MTRR Memory Type | PAT Entry Value | Effective Memory Type |
 * +------------------+-----------------+-----------------------+
 * | UC               | UC              | UC(1)                 |
 * |                  +-----------------+-----------------------+
 * |                  | UC-             | UC(1)                 |
 * |                  +-----------------+-----------------------+
 * |                  | WC              | WC                    |
 * |                  +-----------------+-----------------------+
 * |                  | WT              | UC                    |
 * |                  +-----------------+-----------------------+
 * |                  | WB              | UC(1)                 |
 * |                  +-----------------+-----------------------+
 * |                  | WP              | UC(1)                 |
 * +------------------+-----------------+-----------------------+
 * | WC               | UC              | UC(1)                 |
 * |                  +-----------------+-----------------------+
 * |                  | UC-             | WC                    |
 * |                  +-----------------+-----------------------+
 * |                  | WC              | WC                    |
 * |                  +-----------------+-----------------------+
 * |                  | WT              | UC(2,3)               |
 * |                  +-----------------+-----------------------+
 * |                  | WB              | WC                    |
 * |                  +-----------------+-----------------------+
 * |                  | WP              | UC(2,3)               |
 * +------------------+-----------------+-----------------------+
 * | WT               | UC              | UC(2)                 |
 * |                  +-----------------+-----------------------+
 * |                  | UC-             | UC(2)                 |
 * |                  +-----------------+-----------------------+
 * |                  | WC              | WC                    |
 * |                  +-----------------+-----------------------+
 * |                  | WT              | WT                    |
 * |                  +-----------------+-----------------------+
 * |                  | WB              | WT                    |
 * |                  +-----------------+-----------------------+
 * |                  | WP              | WP(3)                 |
 * +------------------+-----------------+-----------------------+
 * | WB               | UC              | UC(2)                 |
 * |                  +-----------------+-----------------------+
 * |                  | UC-             | UC(2)                 |
 * |                  +-----------------+-----------------------+
 * |                  | WC              | WC                    |
 * |                  +-----------------+-----------------------+
 * |                  | WT              | WT                    |
 * |                  +-----------------+-----------------------+
 * |                  | WB              | WB                    |
 * |                  +-----------------+-----------------------+
 * |                  | WP              | WP                    |
 * +------------------+-----------------+-----------------------+
 * | WP               | UC              | UC(2)                 |
 * |                  +-----------------+-----------------------+
 * |                  | UC-             | WC(3)                 |
 * |                  +-----------------+-----------------------+
 * |                  | WC              | WC                    |
 * |                  +-----------------+-----------------------+
 * |                  | WT              | WT(3)                 |
 * |                  +-----------------+-----------------------+
 * |                  | WB              | WP                    |
 * |                  +-----------------+-----------------------+
 * |                  | WP              | WP                    |
 * +------------------+-----------------+-----------------------+
 */
static enum page_cache_mode pcm = _PAGE_CACHE_MODE_WB; 
static int memory_type = MTRR_MEMTYPE_WB;

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	/* Clear PCD/PAT/PWT */
	pgprot_val(vma->vm_page_prot) &= ~(_PAGE_PCD | _PAGE_PWT | _PAGE_PAT);

	/* _PAGE_PAT: _PAGE_PCD: _PAGE_PWT */
	pgprot_val(vma->vm_page_prot) |= cachemode2protval(pcm);

	return remap_pfn_range(vma, vma->vm_start,
			MTRR_MEM_BASE >> PAGE_SHIFT, PAGE_SIZE,
					vma->vm_page_prot);
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
	/* MTRRs */
	mtrr_add(MTRR_MEM_BASE, MTRR_MEM_SIZE, memory_type, true);

	misc_register(&BiscuitOS_drv);
	return 0;
}
device_initcall(BiscuitOS_init);
