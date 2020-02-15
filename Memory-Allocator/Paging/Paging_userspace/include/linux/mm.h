#ifndef _BISCUITOS_MM_H
#define _BISCUITOS_MM_H

#include "linux/biscuitos.h"

#define PAGE_SHIFT	12
#define PAGE_SIZE	(1 << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

#define PAGE_ALIGN(addr)	ALIGN(addr, PAGE_SIZE)
#define PAGE_ALIGNED(addr)	IS_ALIGNED((unsigned long)(addr), PAGE_SIZE)

typedef u32 pteval_t;
typedef u32 pmdval_t;

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

#define KM_TYPE_NR	16

enum fixed_addresses {
	FIX_EARLYCON_MEM_BASE,
	__end_of_permanent_fixed_addresses,

	FIX_KMAP_BEGIN = __end_of_permanent_fixed_addresses,
	FIX_KMAP_END = FIX_KMAP_BEGIN + (KM_TYPE_NR * NR_CPUS) - 1,

	/* Support writting RO kernel text via kprobes, jump labels, etc. */
	FIX_TEXT_POKE0,
	FIX_TEXT_POKE1,

	__end_of_fixmap_region,

	/*
	 * Share the kmap() region with early_ioremap(): this is guaranteed
	 * not to clash since early_ioremap() is only available before
	 * paging_init(), and kmap() only after.
	 */
#define NR_FIX_BITMAPS		32
#define FIX_BITMAPS_SLOTS	7
#define TOTAL_FIX_BITMAPS	(NR_FIX_BITMAPS * FIX_BITMAPS_SLOTS)

	FIX_BITMAP_END = __end_of_permanent_fixed_addresses,
	FIX_BITMAP_BEGIN = FIX_BITMAP_END + TOTAL_FIX_BITMAPS - 1,
	__end_of_early_ioremap_region
};

static const enum fixed_addresses __end_of_fixed_addresses =
	__end_of_fixmap_region > __end_of_early_ioremap_region ?
	__end_of_fixmap_region : __end_of_early_ioremap_region;

struct mm_struct {
	struct {
		pgd_t *pgd;
	};
};
extern struct mm_struct init_mm;

struct mem_type {
	pteval_t prot_pte;
	pteval_t prot_pte_s2;
	pmdval_t prot_l1;
	pmdval_t prot_sect;
	unsigned int domain;
};

/*
 * Architecture ioremap implementation
 */
#define MT_DEVICE		0
#define MT_DEVICE_NONSHARED	1
#define MT_DEVICE_CACHED	2
#define MT_DEVICE_WC		3

struct map_desc {
	unsigned long virtual;
	unsigned long pfn;
	unsigned long length;
	unsigned int type;
};

/* types 0-3 are defined in asm/io.h */
enum {
	MT_UNCACHED = 4,
	MT_CACHECLEAN,
	MT_MINICLEAN,
	MT_LOW_VECTORS,
	MT_HIGH_VECTORS,
	MT_MEMORY_RWX,
	MT_MEMORY_RW,
	MT_ROM,
	MT_MEMORY_RWX_NONCACHED,
	MT_MEMORY_RW_DTCM,
	MT_MEMORY_RWX_ITCM,
	MT_MEMORY_RW_SO,
	MT_MEMORY_DMA_READY,
};

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
#define PMD_PXNTABLE		(_AT(pmdval_t, 1) << 2)		/* v7 */
#define PMD_BIT4		(_AT(pmdval_t, 1) << 4)
#define PMD_DOMAIN(x)		(_AT(pmdval_t, (x)) << 5)

/*
 * - section
 */
#define PMD_SECT_PXN		(_AT(pmdval_t, 1) << 0)		/* v7 */
#define PMD_SECT_BUFFERABLE	(_AT(pmdval_t, 1) << 2)
#define PMD_SECT_CACHEABLE	(_AT(pmdval_t, 1) << 3)
#define PMD_SECT_XN		(_AT(pmdval_t, 1) << 4)		/* v6 */
#define PMD_SECT_AP_WRITE	(_AT(pmdval_t, 1) << 10)
#define PMD_SECT_AP_READ	(_AT(pmdval_t, 1) << 11)
#define PMD_SECT_TEX(x)		(_AT(pmdval_t, (x)) << 12)	/* v5 */
#define PMD_SECT_APX		(_AT(pmdval_t, 1) << 15)	/* v6 */
#define PMD_SECT_S		(_AT(pmdval_t, 1) << 16)	/* v6 */
#define PMD_SECT_nG		(_AT(pmdval_t, 1) << 17)	/* v6 */
#define PMD_SECT_SUPER		(_AT(pmdval_t, 1) << 18)	/* v6 */
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
#define L_PTE_VALID		(_AT(pteval_t, 1) << 0)		/* Valid */
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

#define PHYS_MASK		(~0UL)

/* to find an entry in a page-table-directory */
#define pgd_index(addr)		((addr) >> PGDIR_SHIFT)

#define pgd_offset(mm, addr)	((mm)->pgd + pgd_index(addr))

/* to find an entry in a kernel page-table-directory */
#define pgd_offset_k(addr)	pgd_offset(&init_mm, addr)

#define pte_index(addr)		(((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
#define pte_pfn(pte)		((pte_val(pte) & PHYS_MASK) >> PAGE_SHIFT)

static inline pud_t *pud_offset(pgd_t *pgd, unsigned long address)
{
	return (pud_t *)pgd;
}

static inline pmd_t *pmd_offset(pud_t *pud, unsigned long addr)
{
	return (pmd_t *)pud;
}

static inline pmd_t *pmd_off_k(unsigned long virt)
{
	return pmd_offset(pud_offset(pgd_offset_k(virt), virt), virt);
}

#define pmd_clear(pmdp)			\
	do {				\
		pmdp[0] = __pmd(0);	\
		pmdp[1] = __pmd(0);	\
	} while (0)

/* we don't need complex calculations here as the pmd is folded into the pgd */
#define pmd_addr_end(addr, end)	(end)
#define pud_addr_end(addr, end)	(end)

/*
 * When walking page table, get the address of the next boundary,
 * or the end address of the range if that comes earlier. Although no
 * vma end wraps to 0, rounded up __boundary may wrap to 0 throught.
 */

#define pgd_addr_end(addr, end)						\
({	unsigned long __boundary = ((addr) + PGDIR_SIZE) & PGDIR_MASK;	\
	(__boundary - 1 < (end) - 1) ? __boundary : (end);		\
})

#define pmd_bad(pmd)		(pmd_val(pmd) & 2)
#define pmd_none(pmd)		(!pmd_val(pmd))

#define pte_none(pte)		(!pte_val(pte))
#define set_pte_ext(ptep,pte,ext)	

/* Phys and virutal */
extern const char *memory;
extern void *high_memory;

#define PAGE_OFFSET		((unsigned long)memory)
#define MAX_VIRTUAL_ADDR	(~((unsigned long)0))

#define FIXADDR_START		0xffc00000UL
#define FIXADDR_END		0xfff00000UL
#define FIXADDR_TOP		(FIXADDR_END - PAGE_SIZE)

/*
 * Just any arbitrary offset to the start of the vmalloc VM area: the
 * current 8MB value just means that there will be a 8MB "hole" after the
 * physical memory until the kernel virtual memory starts. That means that
 * any out-of-bounds memory accesses will hopefully be caught.
 * The vmalloc() routines leaves a hole of 4kB between each vmalloced
 * area for the same reason. ;)
 */
#define VMALLOC_OFFSET		(8*1024*1024)
#define VMALLOC_START		(((unsigned long)high_memory + \
				   VMALLOC_OFFSET) & ~(VMALLOC_OFFSET-1))
#define VMALLOC_END		0xff800000UL

#define SZ_16M			0x01000000

#define PFN_ALIGN(x)	(((unsigned long)(x) + (PAGE_SIZE - 1)) & PAGE_MASK)
#define PFN_UP(x)	(((x) + PAGE_SIZE-1) >> PAGE_SHIFT)
#define PFN_DOWN(x)	((x) >> PAGE_SHIFT)
#define PFN_PHYS(x)	((phys_addr_t)(x) << PAGE_SHIFT)
#define PHYS_PFN(x)	((unsigned long)((x) >> PAGE_SHIFT))

#define __fix_to_virt(x)	(FIXADDR_TOP - ((x) << PAGE_SHIFT))
#define __virt_to_fix(x)	((FIXADDR_TOP - ((x)&PAGE_MASK)) >> PAGE_SHIFT)

static inline unsigned long fix_to_virt(const unsigned int idx)
{
	if (idx >= __end_of_fixed_addresses)
		printk("BUG_ON(): %s\n", __func__);
	return __fix_to_virt(idx);
}

static inline unsigned long virt_to_fix(const unsigned long vaddr)
{
	if (vaddr >= FIXADDR_TOP || vaddr < FIXADDR_START)
		printk("BUG_ON(): %s\n", __func__);
	return __virt_to_fix(vaddr);
}

static inline phys_addr_t virt_to_phys(const volatile void *x)
{
	return ((unsigned long)(x) - PAGE_OFFSET) + CONFIG_MEMBLOCK_BASE;
}

static inline void *phys_to_virt(phys_addr_t x)
{
	return (void *)(((unsigned long)(x) - CONFIG_MEMBLOCK_BASE) +
					PAGE_OFFSET);
}

/*
 * Drivers should NOT use these either.
 */
#define __pa(x)		virt_to_phys((void *)(x))
#define __va(x)		phys_to_virt((phys_addr_t)(x))

#define PHYS_OFFSET		__pa(PAGE_OFFSET)
#define PG_DIR_SIZE		0x4000

/* Emulate MMU OFFSET */
#define MMU_PHYS_OFFSET		PHYS_OFFSET
#define MMU_PAGE_OFFSET		(PHYS_OFFSET + CONFIG_PAGE_OFFSET)

/*
 * TASK_SIZE - the maximum size of a use space task.
 */
#define TASK_SIZE		(MMU_PAGE_OFFSET - SZ_16M)

/*
 * The module space lives between the addresses given be TASK_SIZE
 * and PAGE_OFFSET - it must be within 32MB of the kernel text.
 */
#define MODULES_VADDR		(MMU_PAGE_OFFSET - SZ_16M)
/*
 * The highmem pkmap virtual space shares the end of the module area.
 */
#define MODULES_END		(MMU_PAGE_OFFSET)

/*
 * Covert a physical address to a Page Frame Number and back
 */
#define __phys_to_pfn(paddr)	PHYS_PFN(paddr)
#define __pfn_to_phys(pfn)	PFN_PHYS(pfn)

#define vectors_base()		0xffff0000

#define __mmu_phys_to_virt(x)	((unsigned long)(x) + \
				 (unsigned long)MMU_PAGE_OFFSET - \
				 (unsigned long)MMU_PHYS_OFFSET)
#define __mmu_virt_to_phys(x)	((unsigned long)(x) - \
				 (unsigned long)MMU_PAGE_OFFSET + \
				 (unsigned long)MMU_PHYS_OFFSET)
#define __mmu_va(x)		((void *)__mmu_phys_to_virt((phys_addr_t)(x)))
#define __mmu_pa(x)		__mmu_virt_to_phys((unsigned long)(x))

static inline pte_t *pmd_page_vaddr(pmd_t pmd)
{
	return __mmu_va((unsigned long)pmd_val(pmd) & 
			(unsigned long)PHYS_MASK & (unsigned long)PAGE_MASK);
}

#define pte_index(addr)		(((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))

#define pte_offset_kernel(pmd, addr)	(pmd_page_vaddr(*(pmd)) + \
							pte_index(addr))
/* Emulate Kernel Image */
#define KERNEL_START		(PAGE_OFFSET + 0x100000)
#define KERNEL_END		(KERNEL_START + 2 * SECTION_SIZE)

extern void page_table_init(void);
extern void dup_pgdir(void);
#endif
