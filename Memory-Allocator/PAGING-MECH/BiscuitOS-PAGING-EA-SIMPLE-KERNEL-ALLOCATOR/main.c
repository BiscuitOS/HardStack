// SPDX-License-Identifier: GPL-2.0
/*
 * PAGING Project: Simpler Kernel ALLOCATOR
 *
 * (C) 2023.11.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>

struct vpage {
	unsigned long vaddr;
	bool inuse;
	struct list_head lru;
};

static struct kmem_cache *vpage_cache;
static LIST_HEAD(vpage_free_head);
static LIST_HEAD(vpage_inuse_head);

static int tmap_mapping(struct vpage *vpage, phys_addr_t phys, pgprot_t prot)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	/* PGD */
	pgd = pgd_offset_k(vpage->vaddr);
	if (pgd_bad(*pgd))
		return -EINVAL;
	/* P4D */
	if (unlikely(pgd_none(*pgd)))
		__p4d_alloc(&init_mm, pgd, vpage->vaddr);
	p4d = p4d_offset(pgd, vpage->vaddr); 
	/* PUD */
	if (unlikely(p4d_none(*p4d)))
		__pud_alloc(&init_mm, p4d, vpage->vaddr);
	pud = pud_offset(p4d, vpage->vaddr);
	/* PMD */
	if (unlikely(pud_none(*pud)))
		__pmd_alloc(&init_mm, pud, vpage->vaddr);
	pmd = pmd_offset(pud, vpage->vaddr);
	/* PTE */
	if (unlikely(pmd_none(*pmd)))
		__pte_alloc_kernel(pmd);
	pte = pte_offset_kernel(pmd, vpage->vaddr);

	/* SET PGTABLE */
	set_pte_at(&init_mm, vpage->vaddr, pte,
			pte_mkwrite(pfn_pte(phys >> PAGE_SHIFT, prot)));
	
	return 0;
}

static void *_tmap_prot(phys_addr_t phys, pgprot_t prot)
{
	struct vpage *vpage;

	if (list_empty(&vpage_free_head))
		return NULL;

	vpage = list_first_entry(&vpage_free_head, struct vpage, lru);

	tmap_mapping(vpage, phys, prot);

	vpage->inuse = true;
	list_move(&vpage->lru, &vpage_inuse_head);

	return (void *)vpage->vaddr;
}

static int tump_pte(pte_t *pte, unsigned long addr, void *data)
{
	pte_clear(&init_mm, addr, pte);

	return 0;
}

static void _tumap(struct vpage *vpage)
{
	apply_to_page_range(&init_mm, vpage->vaddr, PAGE_SIZE, tump_pte, NULL);
}

void BiscuitOS_tumap(void *addr)
{
	struct vpage *vpage, *next;

	list_for_each_entry_safe(vpage, next, &vpage_inuse_head, lru)
		if ((unsigned long)addr == vpage->vaddr) {
			_tumap(vpage);
			list_move_tail(&vpage->lru, &vpage_free_head);
		}
}
EXPORT_SYMBOL_GPL(BiscuitOS_tumap);

void *BiscuitOS_tmap(phys_addr_t phys)
{
	return _tmap_prot(phys, PAGE_KERNEL);
}
EXPORT_SYMBOL_GPL(BiscuitOS_tmap);

void *BiscuitOS_tmap_io(phys_addr_t phys)
{
	return _tmap_prot(phys, PAGE_KERNEL_IO);
}
EXPORT_SYMBOL_GPL(BiscuitOS_tmap_io);

static int __vpage_init(unsigned long start_pfn, unsigned long end_pfn)
{
	unsigned long vstart, vend;
	unsigned long _start, _end;
	unsigned long index;
	
	vstart = (unsigned long)__va(start_pfn << PAGE_SHIFT);
	vend   = (unsigned long)__va(end_pfn << PAGE_SHIFT);

	_start = PAGE_ALIGN(vstart);
	_end   = PAGE_ALIGN_DOWN(vend);

	if (_start > _end)
		return 0;

	for (index = _start; index < _end; index += PAGE_SIZE) {
		struct vpage *vpage = kmem_cache_alloc(vpage_cache, GFP_KERNEL);

		if (!vpage)
			return -ENOMEM;

		vpage->vaddr = index;
		vpage->inuse = false;
		list_add(&vpage->lru, &vpage_free_head);
	}

	return 0;
}

static int vpage_alloc_init(void)
{
	int i, j;

	for (i = 0, j = 1; j < nr_pfn_mapped; i++, j++)
		if (pfn_mapped[j].start > pfn_mapped[i].end)
			__vpage_init(pfn_mapped[i].end, pfn_mapped[j].start);

	return 0;
}

static int __init BiscuitOS_init(void)
{
	char *buffer;

	vpage_cache = kmem_cache_create("BiscuitOS-VPAGE-ALLOCTOR",
				sizeof(struct vpage), 0, 0, NULL);
	if (!vpage_cache)
		return -ENOMEM;

	vpage_alloc_init();

	buffer = BiscuitOS_tmap(0x10000000);
	sprintf(buffer, "Hello BiscuitOS");
	printk("%s", buffer);

	BiscuitOS_tumap(buffer);
	return 0;
}
__initcall(BiscuitOS_init);
