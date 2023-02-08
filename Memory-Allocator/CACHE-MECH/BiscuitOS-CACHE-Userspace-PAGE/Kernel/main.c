/*
 * CACHE Mode for Userspace Page on BiscuitOS
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
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/memtype.h>

#define DEV_NAME			"BiscuitOS-CACHE"
/* IOCTL */
#define BISCUITOS_CACHE_IO		0xBD
#define BSIO_CACHE_MODE_WB		_IO(BISCUITOS_CACHE_IO, 0x00)
#define BSIO_CACHE_MODE_WC		_IO(BISCUITOS_CACHE_IO, 0x01)
#define BSIO_CACHE_MODE_UC_MINUS	_IO(BISCUITOS_CACHE_IO, 0x02)
#define BSIO_CACHE_MODE_UC		_IO(BISCUITOS_CACHE_IO, 0x03)
#define BSIO_CACHE_MODE_WT		_IO(BISCUITOS_CACHE_IO, 0x04)
#define BSIO_CACHE_MODE_WP		_IO(BISCUITOS_CACHE_IO, 0x05)
/* page memory type */
static enum page_cache_mode page_pcm;

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;
	enum page_cache_mode new_pcm;
	struct page *fault_page;
	resource_size_t phys;
	int r;

	/* Clear PAT */
	pgprot_val(vma->vm_page_prot) &= ~(_PAGE_PWT | _PAGE_PCD | _PAGE_PAT);
	/* Setup PAT */
	pgprot_val(vma->vm_page_prot) |= cachemode2protval(page_pcm);

	/* Allocate Physical Memory */
	fault_page = alloc_page(GFP_KERNEL);
	if (!fault_page) {
		r = -ENOMEM;
		goto alloc_err;
	}
	phys = page_to_pfn(fault_page) << PAGE_SHIFT;

	/* Physical Memory Type */
	r = memtype_reserve(phys, phys + PAGE_SIZE, page_pcm, &new_pcm);
	if (r || page_pcm != new_pcm) {
		r = -EINVAL;
		printk("Incorrect memory type!\n");
		goto type_err;
	}

	/* Fill PTE and INC _mapcount for page */
	vma->vm_flags |= VM_MIXEDMAP;
	if (vm_insert_page(vma, address, fault_page)) {
		r = -EAGAIN;
		goto map_err;
	}

	/* Add refcount for page */
	atomic_inc(&fault_page->_refcount);
	/* Bind fault page */
	vmf->page = fault_page;
	printk("Phys: %#llx - %#llx\n", phys, phys + PAGE_SIZE);

	return 0;

map_err:
	memtype_free(phys, phys + PAGE_SIZE);
type_err:
	__free_page(fault_page);
alloc_err:
	return r;
}

static long BiscuitOS_ioctl(struct file *filp,
				unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BSIO_CACHE_MODE_WB:
		page_pcm = _PAGE_CACHE_MODE_WB;
		break;
	case BSIO_CACHE_MODE_WC:
		page_pcm = _PAGE_CACHE_MODE_WC;
		break;
	case BSIO_CACHE_MODE_UC_MINUS:
		page_pcm = _PAGE_CACHE_MODE_UC_MINUS;
		break;
	case BSIO_CACHE_MODE_UC:
		page_pcm = _PAGE_CACHE_MODE_UC;
		break;
	case BSIO_CACHE_MODE_WT:
		page_pcm = _PAGE_CACHE_MODE_WT;
		break;
	case BSIO_CACHE_MODE_WP:
		page_pcm = _PAGE_CACHE_MODE_WP;
		break;
	default:
		printk("Unknown Page memory type!\n");
		return -EINVAL;
	}
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
	.unlocked_ioctl = BiscuitOS_ioctl,
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

device_initcall(BiscuitOS_init);
