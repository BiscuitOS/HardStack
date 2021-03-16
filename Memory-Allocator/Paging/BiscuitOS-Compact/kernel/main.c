/*
 * Memory Compact Mechanism on BiscuitOS
 *
 * (C) 2021.03.16 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/bitmap.h>
#include <linux/kallsyms.h>
#include <linux/crypto.h>

#define MODULE_NAME			"BiscuitOS"
/* IOCTL */
#define BISCUITOS_IO			0xBD
#define BISCUITOS_COMPRESS		_IO(BISCUITOS_IO, 0x00)

/* Compact */
#define BISCUITOS_CRYPTO		"lzo"
#define BISCUITOS_COMPACT_SIZE		(8 * 1024 * 1024)
#define BISCUITOS_COMPACT_BUCK		64
#define BISCUITOS_PER_BLOCKS		(PAGE_SIZE / BISCUITOS_COMPACT_BUCK)
#define BISCUITOS_COMPACT_NR		(BISCUITOS_COMPACT_SIZE / \
					 BISCUITOS_COMPACT_BUCK)

/* Compact Entry */
struct BiscuitOS_compact_socket {
	struct vm_area_struct	*vma;
	unsigned long		address;
	struct list_head	list;
	unsigned long		socket;
	unsigned long		blocks;
};
static LIST_HEAD(BiscuitOS_compact_list);
static DECLARE_BITMAP(BiscuitOS_Bitmap, BISCUITOS_COMPACT_NR / BITS_PER_LONG);
static struct kmem_cache *BiscuitOS_compact_socket_cache;
static DEFINE_SPINLOCK(BiscuitOS_compact_lock);
static void *BiscuitOS_Compact_Space;

/* BiscuitOS page */
struct BiscuitOS_page {
	struct vm_area_struct	*vma;
	unsigned long		address;
	struct page		*page;
	struct list_head	list;
	pte_t			*pte;
};
static LIST_HEAD(BiscuitOS_page_list);
static struct kmem_cache *BiscuitOS_page_cache;
static DEFINE_SPINLOCK(BiscuitOS_page_list_lock);

/* Module and kallsyms */
typedef void (*flush_tlb_t)(struct mm_struct *mm, unsigned long start,
		unsigned long end, unsigned int stride_shift, bool freed);
static flush_tlb_t flush_tlb_mm_range_func;

/* Memory Compress Routine */
static int BiscuitOS_compress(struct BiscuitOS_page *Bpage)
{
	struct vm_area_struct *vma = Bpage->vma;
	struct BiscuitOS_compact_socket *socket;
	struct crypto_comp *tfm;
	unsigned long index;
	unsigned long blocks;
	unsigned int dlen;
	void *src, *dst;

	/* Search valid compact socket */
	index = bitmap_find_next_zero_area_off(BiscuitOS_Bitmap,
			BISCUITOS_COMPACT_NR, 0, BISCUITOS_PER_BLOCKS, 1, 0);
	BUG_ON(index > BISCUITOS_COMPACT_NR);
	dst = (void *)((unsigned long)BiscuitOS_Compact_Space + 
					index * BISCUITOS_COMPACT_BUCK);

	/* Compact lib */
	tfm = crypto_alloc_comp(BISCUITOS_CRYPTO, 0, 0);

	/* Compress */
	src = kmap_atomic(Bpage->page);
	crypto_comp_crt(tfm)->cot_compress(crypto_comp_tfm(tfm),
						src, PAGE_SIZE, dst, &dlen);
	kunmap_atomic(src);

	/* Update Compact Space information */
	dlen += BISCUITOS_COMPACT_BUCK - 1;
	blocks = dlen / BISCUITOS_COMPACT_BUCK;
	bitmap_set(BiscuitOS_Bitmap, index, blocks);

	/* Update PTE */
	Bpage->pte->pte = 0;
	flush_tlb_mm_range_func(vma->vm_mm, vma->vm_start, 
					vma->vm_end, PAGE_SHIFT, false);
	__free_page(Bpage->page);

	/* Update Compact socket */
	socket = kmem_cache_zalloc(BiscuitOS_compact_socket_cache, GFP_KERNEL);
	if (!socket) {
		printk("ERROR: allocate BiscuitOS compact socket.\n");
		return -ENOMEM;
	}
	spin_lock(&BiscuitOS_compact_lock);
	socket->address = Bpage->address;
	socket->vma = Bpage->vma;
	socket->socket = index;
	socket->blocks = blocks;
	list_add(&socket->list, &BiscuitOS_compact_list);
	spin_unlock(&BiscuitOS_compact_lock);
	dec_mm_counter(vma->vm_mm, MM_FILEPAGES);

	return 0;
}

/* Decompress Routine */
static int BiscuitOS_decompress(struct BiscuitOS_page *Bpage)
{
	struct BiscuitOS_compact_socket *socket, *tmp;
	struct crypto_comp *tfm;
	unsigned int slen, dlen;
	void *src, *dst;

	/* Find Compact Socket */
	list_for_each_entry(tmp, &BiscuitOS_compact_list, list) {
		if (tmp->address == Bpage->address && tmp->vma == Bpage->vma) {
			socket = tmp;
			break;
		} else
			socket = NULL;
	}

	if (!socket)
		return -EINVAL;

	/* Compact lib */
	tfm = crypto_alloc_comp(BISCUITOS_CRYPTO, 0, 0);

	/* Decompress */
	src = (void *)((unsigned long)BiscuitOS_Compact_Space +
				socket->socket * BISCUITOS_COMPACT_BUCK);
	slen = socket->blocks * BISCUITOS_COMPACT_BUCK;
	dlen = PAGE_SIZE;
	dst = kmap_atomic(Bpage->page);
	crypto_comp_crt(tfm)->cot_decompress(crypto_comp_tfm(tfm),
						src, slen, dst, &dlen);
	kunmap_atomic(dst);

	/* Update Compact socket */
	bitmap_clear(BiscuitOS_Bitmap, socket->socket, socket->blocks);
	list_del(&socket->list);
	kmem_cache_free(BiscuitOS_compact_socket_cache, socket);

	return 0;
}

/* follow pte */
static int BiscuitOS_get_locked_pte(struct mm_struct *mm, 
		unsigned long address, pte_t **ptep, spinlock_t **ptl)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	/* Follow PGD Entry */
	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		goto out;

	/* Follow P4D Entry */
	p4d = p4d_offset(pgd, address);
	if (p4d_none(*p4d) || unlikely(p4d_bad(*p4d)))
		goto out;

	/* Follow PUD Entry */
	pud = pud_offset(p4d, address);
	if (pud_none(*pud) || unlikely(pud_bad(*pud)))
		goto out;

	/* Follow PMD Entry */
	pmd = pmd_offset(pud, address);
	if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd)))
		goto out;

	/* Follow PTE */
	pte = pte_offset_map_lock(mm, pmd, address, ptl);
	if (!pte)
		goto out;
	if (!pte_present(*pte))
		goto unlock;
	*ptep = pte;
	return 0;

unlock:
	pte_unmap_unlock(ptep, *ptl);
out:
	return -EINVAL;
}

/* Page Fault */
static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;
	struct BiscuitOS_page *Bpage;
	struct page *fault_page;
	spinlock_t *ptl = NULL;
	pte_t *pte = NULL;
	int r;

	/* Allocate new page from buddy */
	fault_page = alloc_page(GFP_KERNEL);
	if (!fault_page) {
		printk("ERROR: System doesn't has enough physical memory.\n");
		r = -ENOMEM;
		goto err_alloc;
	}

	/* BiscuitOS page */
	Bpage = kmem_cache_zalloc(BiscuitOS_page_cache, GFP_KERNEL);
	if (!Bpage) {
		printk("ERROR: fault kmem cache.\n");
		r = -ENOMEM;
		goto err_kmem;
	}

	/* Establish pte and INC _mapcount for page */
	vma->vm_flags |= VM_MIXEDMAP;
	if (vm_insert_page(vma, address, fault_page))
		return -EAGAIN;

	/* Add refcount for page */
	atomic_inc(&fault_page->_refcount);
	/* bind fault page */
	vmf->page = fault_page;

	/* Special pte */
	BiscuitOS_get_locked_pte(vma->vm_mm, address, &pte, &ptl);
	if (pte && pte_present(*pte)) {
		/* Add to page cache */
		spin_lock(&BiscuitOS_page_list_lock);
		list_add(&Bpage->list, &BiscuitOS_page_list);
		Bpage->page = fault_page;
		Bpage->vma = vma;
		Bpage->pte = pte;
		Bpage->address = address;
		spin_unlock(&BiscuitOS_page_list_lock);
		pte_unmap_unlock(pte, ptl);
	}

	/* Decompress */
	spin_lock(&BiscuitOS_compact_lock);
	BiscuitOS_decompress(Bpage);
	spin_unlock(&BiscuitOS_compact_lock);

	return 0;

err_kmem:
	__free_page(fault_page);
err_alloc:
	return r;
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

static long BiscuitOS_ioctl(struct file *filp,
			unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BISCUITOS_COMPRESS: {
		struct BiscuitOS_page *Bpage, *next;

		spin_lock(&BiscuitOS_page_list_lock);
		list_for_each_entry_safe(Bpage, next, 
						&BiscuitOS_page_list, list) {
			BiscuitOS_compress(Bpage);
			list_del(&Bpage->list);
			kmem_cache_free(BiscuitOS_page_cache, Bpage);
		}

		spin_unlock(&BiscuitOS_page_list_lock);
		break;
	}
	default:
		break;
	}
	return 0;
}

static int __init kallsyms_module_init(void)
{
	flush_tlb_mm_range_func = 
		(flush_tlb_t)kallsyms_lookup_name("flush_tlb_mm_range");
	if (!flush_tlb_mm_range_func)
		return -EINVAL;

	return 0;
}

/* file operations */
static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
	.unlocked_ioctl	= BiscuitOS_ioctl,
};

/* Misc device driver */
static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= MODULE_NAME,
	.fops	= &BiscuitOS_fops,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	int ret;

	/* Register Misc device */
	ret = misc_register(&BiscuitOS_drv);
	if (ret < 0) {
		printk("ERROR: Unable register misc device.\n");
		goto err_misc;
	}

	/* cache for BiscuitOS_page */
	BiscuitOS_page_cache = kmem_cache_create("BiscuitOS_page",
		sizeof(struct BiscuitOS_page), 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!BiscuitOS_page_cache) {
		printk("ERROR: BiscuitOS_page_cache.\n");
		ret = -ENOMEM;
		goto err_page_cache;
	}

	/* cache for BiscuitOS_compact_socket */
	BiscuitOS_compact_socket_cache = kmem_cache_create(
			"BiscuitOS_compact_socket",
			sizeof(struct BiscuitOS_compact_socket),
			0,
			SLAB_HWCACHE_ALIGN,
			NULL);
	if (!BiscuitOS_compact_socket_cache) {
		printk("ERROR: BiscuitOS_compact_socket_cache.\n");
		ret = -ENOMEM;
		goto err_compact_cache;
	}

	/* Compact Space */
	BiscuitOS_Compact_Space = vmalloc(BISCUITOS_COMPACT_SIZE);
	if (!BiscuitOS_Compact_Space) {
		printk("ERROR: BiscuitOS_Compact_Space Alloc.\n");
		ret = -ENOMEM;
		goto err_space;
	}

	ret = kallsyms_module_init();
	if (ret < 0) {
		printk("ERROR: Unknow symbol on module.\n");
		goto err_symbol;
	}

	return 0;

err_symbol:
	vfree(BiscuitOS_Compact_Space);
err_space:
	kmem_cache_destroy(BiscuitOS_compact_socket_cache);
err_compact_cache:
	kmem_cache_destroy(BiscuitOS_page_cache);
err_page_cache:
	misc_deregister(&BiscuitOS_drv);
err_misc:
	return ret;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	vfree(BiscuitOS_Compact_Space);
	kmem_cache_destroy(BiscuitOS_compact_socket_cache);
	kmem_cache_destroy(BiscuitOS_page_cache);
	/* Un-Register Misc device */
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS Memory Compact Mechanism");
