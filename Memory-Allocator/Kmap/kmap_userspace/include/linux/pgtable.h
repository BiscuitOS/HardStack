#ifndef _BISCUITOS_PGTABLE_H
#define _BISCUITOS_PGTABLE_H

#include "linux/buddy.h"

typedef unsigned long phys_addr_t;
typedef unsigned int u32;
typedef u32 pteval_t;
typedef u32 pmdval_t;

#define PHYS_MASK		(~0UL)
#define _AT(T,X)		(X)
#define DOMAIN_KERNEL		2

/*
 * These are used to make use of C type-checking
 */
typedef struct { pteval_t pte; } pte_t;
typedef struct { pmdval_t pmd; } pmd_t;
typedef struct { pmdval_t pgd[2]; } pgd_t;
typedef struct { pteval_t pgprot; } pgprot_t;
typedef struct { pgd_t pgd; } pud_t;

#define pte_val(x)	((x).pte)
#define pmd_val(x)	((x).pmd)
#define pgd_val(x)	((x).pgd[0])
#define pgprot_val(x)	((x).pgprot)

#define __pte(x)	((pte_t) { (x) } )
#define __pmd(x)	((pmd_t) { (x) } )
#define __pgprot(x)	((pgprot_t) { (x) } )

struct mm_struct {
	struct {
		pgd_t *pgd;
	};
};

extern struct mm_struct init_mm;
/*
 * Hardware-wise, we have a two level page table structure, where the first
 * level has 4096 entries, and the second level has 256 entries. Each entry
 * is one 32-bit word. Most of the bits in the second level entry are used
 * by hardware, and there aren't any "accessed" and "dirty" bits.
 *
 * Linux on the other hand has a three level page table structure, which can
 * be wrapped to fit a two level page table structure easily - using the PGD
 * and PTE only. However, Linux also expects one "PTE" table per page, and
 * at least a "dirty" bit.
 *
 * Therefore, we tweak the implementation slightly - we tell Linux that we
 * have 2048 entries in the first level, each of which is 8 bytes (iow, two
 * hardware pointers to the second level.) The second level contains two
 * hardware PTE table arranged contiguously, preceded by Linux versions
 * which contain the state information Linux needs. We, therefore, end up
 * with 512 entries in the "PTE" level.
 *
 * This leads to the page tables having the following layout:
 *
 *     pgd
 * |        |        +------------+ +0
 * +--------+        | Linux pt 0 |
 * |        |        +------------+ +1024
 * +--------+ +0     | Linux pt 1 |
 * |        | -----> +------------+ +2048
 * +--------+ +4     |  h/w pt 0  |
 * |        | -----> +------------+ +3072
 * +--------+ +8     |  h/w pt 1  |
 * |        |        +------------+ +4096
 * +--------+
 */
#define PTRS_PER_PTE		512
#define PTRS_PER_PMD		1
#define PTRS_PER_PGD		2048
#define PG_DIR_SIZE		0x4000

#define PTE_HWTABLE_PTRS	(PTRS_PER_PTE)
#define PTE_HWTABLE_OFF		(PTE_HWTABLE_PTRS * sizeof(pte_t))
#define PTE_HWTABLE_SIZE	(PTRS_PER_PTE * sizeof(u32))

/*
 * PMD_SHIFT determines the size of the area a second-level page table can
 * map PGDIR_SHIFT determines what a third-level page table entry can map.
 */
#define PMD_SHIFT		21
#define PGDIR_SHIFT		21

#define PMD_SIZE		(1UL << PMD_SHIFT)
#define PMD_MASK		(~(PMD_SIZE-1))
#define PGDIR_SIZE		(1UL << PGDIR_SHIFT)
#define PGDIR_MASK		(~(PGDIR_SIZE-1))

#define PUD_SHIFT		PGDIR_SHIFT
#define PUD_SIZE		(1UL << PUD_SHIFT)
#define PUD_MASK		(~(PUD_SIZE-1))

/*
 * Hardware page table definitions
 *
 * + Level 1 descriptor (PMD)
 *   - common
 */
#define PMD_TYPE_MASK		(_AT(pmdval_t, 3) << 0)
#define PMD_TYPE_FAULT		(_AT(pmdval_t, 0) << 0)
#define PMD_TYPE_TABLE		(_AT(pmdval_t, 1) << 0)
#define PMD_TYPE_SECT		(_AT(pmdval_t, 2) << 0)
#define PMD_PXNTABLE		(_AT(pmdval_t, 1) << 2)         /* v7 */
#define PMD_BIT4		(_AT(pmdval_t, 1) << 4)
#define PMD_DOMAIN(x)		(_AT(pmdval_t, (x)) << 5)

/*
 * - section
 */
#define PMD_SECT_PXN		(_AT(pmdval_t, 1) << 0)         /* v7 */
#define PMD_SECT_BUFFERABLE	(_AT(pmdval_t, 1) << 2)
#define PMD_SECT_CACHEABLE	(_AT(pmdval_t, 1) << 3)
#define PMD_SECT_XN		(_AT(pmdval_t, 1) << 4)         /* v6 */
#define PMD_SECT_AP_WRITE	(_AT(pmdval_t, 1) << 10)
#define PMD_SECT_AP_READ	(_AT(pmdval_t, 1) << 11)
#define PMD_SECT_TEX(x)		(_AT(pmdval_t, (x)) << 12)      /* v5 */
#define PMD_SECT_APX		(_AT(pmdval_t, 1) << 15)        /* v6 */
#define PMD_SECT_S		(_AT(pmdval_t, 1) << 16)        /* v6 */
#define PMD_SECT_nG		(_AT(pmdval_t, 1) << 17)        /* v6 */
#define PMD_SECT_SUPER		(_AT(pmdval_t, 1) << 18)        /* v6 */
#define PMD_SECT_AF		(_AT(pmdval_t, 0))

#define PMD_SECT_UNCACHED	(_AT(pmdval_t, 0))
#define PMD_SECT_BUFFERED	(PMD_SECT_BUFFERABLE)
#define PMD_SECT_WT		(PMD_SECT_CACHEABLE)
#define PMD_SECT_WB		(PMD_SECT_CACHEABLE | PMD_SECT_BUFFERABLE)
#define PMD_SECT_MINICACHE	(PMD_SECT_TEX(1) | PMD_SECT_CACHEABLE)
#define PMD_SECT_WBWA		(PMD_SECT_TEX(1) | PMD_SECT_CACHEABLE | \
                                                   PMD_SECT_BUFFERABLE)
#define PMD_SECT_CACHE_MASK	(PMD_SECT_TEX(1) | PMD_SECT_CACHEABLE | \
                                                   PMD_SECT_BUFFERABLE)
#define PMD_SECT_NONSHARED_DEV	(PMD_SECT_TEX(2))
#define PMD_FLAGS_UP		PMD_SECT_WB

#define _PAGE_KERNEL_TABLE	(PMD_TYPE_TABLE | PMD_BIT4 | \
                                 PMD_DOMAIN(DOMAIN_KERNEL))

#define _PAGE_USER_TABLE	(PMD_TYPE_TABLE | PMD_BIT4 | \
                                 PMD_DOMAIN(DOMAIN_KERNEL))

/*
 * section address mask and size definitions.
 */
#define SECTION_SHIFT		20
#define SECTION_SIZE		(1UL << SECTION_SHIFT)
#define SECTION_MASK		(~(SECTION_SIZE-1))

/*
 * "Linux" PTE definitions
 *
 * We keep two sets of PTEs - the hardware and the linux version.
 * This allow greater flexiblilty in the way we map the Linux bits
 * onto the hardware tables, and allows us to have YOUNG and DIRTY
 * bits.
 *
 * The PTE table pointer refers to the hardware entries: the "Linux"
 * entries are stored 1024 bytes below.
 */
#define L_PTE_VALID		(_AT(pteval_t, 1) << 0)         /* Valid */
#define L_PTE_PRESENT		(_AT(pteval_t, 1) << 0)
#define L_PTE_YOUNG		(_AT(pteval_t, 1) << 1)
#define L_PTE_DIRTY		(_AT(pteval_t, 1) << 6)
#define L_PTE_RDONLY		(_AT(pteval_t, 1) << 7)
#define L_PTE_USER		(_AT(pteval_t, 1) << 8)
#define L_PTE_XN		(_AT(pteval_t, 1) << 9)
#define L_PTE_SHARED		(_AT(pteval_t, 1) << 10)
#define L_PTE_NONE		(_AT(pteval_t, 1) << 11)

#define L_PTE_MT_DEV_SHARED	(_AT(pteval_t, 0x04) << 2)
#define L_PTE_MT_MASK		(_AT(pteval_t, 0x0f) << 2)

/** PGD **/

/* to find an entry in a page-table-directory */
#define pgd_index(addr)		((addr) >> PGDIR_SHIFT)

#define pgd_offset(mm, addr)	((mm)->pgd + pgd_index(addr))

/* to find an entry in a kernel page-table-directory */
#define pgd_offset_k(addr)	pgd_offset(&init_mm, addr)

/** PUD **/

static inline pud_t *pud_offset(pgd_t *pgd, unsigned long address)
{
	return (pud_t *)pgd;
}

/** PMD **/

static inline pmd_t *pmd_offset(pud_t *pud, unsigned long addr)
{
	return (pmd_t *)pud;
}

static inline pmd_t *pmd_off_k(unsigned long virt)
{
	return pmd_offset(pud_offset(pgd_offset_k(virt), virt), virt);
}

static inline void __pmd_populate(pmd_t *pmdp, phys_addr_t pte,
							pmdval_t prot)
{
	pmdval_t pmdval = (pte + PTE_HWTABLE_OFF) | prot;
	pmdp[0] = __pmd(pmdval);
}

static inline void pmd_populate_kernel(struct mm_struct *mm,
				pmd_t *pmdp, pte_t *ptep)
{
	__pmd_populate(pmdp, __pa(ptep), _PAGE_KERNEL_TABLE);
}

#define pmd_none(pmd)		(!pmd_val(pmd))

static inline pte_t *pmd_page_vaddr(pmd_t pmd)
{
	return __va(pmd_val(pmd) & PHYS_MASK & PAGE_MASK);
}

/** PTE **/

static inline void set_pte_ext(pte_t *ptep, pte_t pteval)
{
	*ptep = pteval;
}

static inline void set_pte_at(struct mm_struct *mm, unsigned long addr,
			pte_t *ptep, pte_t pteval)
{
	*ptep = pteval;
}

#define pte_pfn(pte)		((pte_val(pte) & PHYS_MASK) >> PAGE_SHIFT)
#define pfn_pte(pfn,prot)	__pte(PFN_PHYS(pfn) | pgprot_val(prot))
#define pte_page(pte)		pfn_to_page(pte_pfn(pte))
#define pte_index(addr)		(((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
#define pte_offset_kernel(pmd, addr)	\
				(pmd_page_vaddr(*(pmd)) + pte_index(addr))
#define pte_clear(mm,addr,ptep)	set_pte_ext(ptep, __pte(0))
#define mk_pte(page,prot)	pfn_pte(page_to_pfn(page), prot)
#define pte_none(pte)		(!pte_val(pte))

#define _MOD_PROT(p, b)		__pgprot(pgprot_val(p) | (b))
extern pgprot_t			pgprot_kernel;

#define PAGE_KERNEL		_MOD_PROT(pgprot_kernel, L_PTE_XN)
#define kmap_prot		PAGE_KERNEL

#endif
