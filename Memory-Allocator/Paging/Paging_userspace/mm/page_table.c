/*
 * Page-Table
 *
 * (C) 2020.02.14 (Valentine's Day) BuddyZhang1 <buddy.zhang@aliyun.com> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "linux/biscuitos.h"
#include "linux/memblock.h"
#include "linux/mm.h"

/* BM PTE-TABLE */
static pte_t *bm_pte;
/* page-dir */
pgd_t *swapper_pg_dir;
/* high memory */
void *high_memory;
/* Default system pgdir valud */
static unsigned long default_pgdir_val = PMD_TYPE_SECT		| \
					 PMD_SECT_AP_WRITE	| \
					 PMD_SECT_AP_READ	| \
					 PMD_SECT_AF		| \
					 PMD_FLAGS_UP;

struct mm_struct init_mm;

static struct mem_type mem_types[] = {
	[MT_MEMORY_RWX] = {
		.prot_pte	= L_PTE_PRESENT | L_PTE_YOUNG | L_PTE_DIRTY,
		.prot_l1	= PMD_TYPE_TABLE,
		.prot_sect	= PMD_TYPE_SECT | PMD_SECT_AP_WRITE,
		.domain		= DOMAIN_KERNEL,
	},
	[MT_MEMORY_RW] = {
		.prot_pte	= L_PTE_PRESENT | L_PTE_YOUNG | L_PTE_DIRTY |
				  L_PTE_XN,
		.prot_l1	= PMD_TYPE_TABLE,
		.prot_sect	= PMD_TYPE_SECT | PMD_SECT_AP_WRITE,
		.domain		= DOMAIN_KERNEL,
	},
	[MT_HIGH_VECTORS] = {
		.prot_pte	= L_PTE_PRESENT | L_PTE_YOUNG | L_PTE_DIRTY |
				  L_PTE_USER | L_PTE_RDONLY,
		.prot_l1	= PMD_TYPE_TABLE,
		.domain		= DOMAIN_VECTORS,
	}
};

static inline pmd_t *fixmap_pmd(unsigned long addr)
{
	pgd_t *pgd = pgd_offset_k(addr);
	pud_t *pud = pud_offset(pgd, addr);
	pmd_t *pmd = pmd_offset(pud, addr);

	return pmd;
}

static inline void __pmd_populate(pmd_t *pmdp, phys_addr_t pte,
							pmdval_t prot)
{
	pmdval_t pmdval = (pte + PTE_HWTABLE_OFF) | prot;
	pmdp[0] = __pmd(pmdval);
}

/*
 * Pupulate the pmdp entry with a pointer to the pte. This pmd is part
 * of the mm address space.
 *
 * Ensure the we always set both PMD entries.
 */
static inline void pmd_populate_kernel(struct mm_struct *mm,
					pmd_t *pmdp, pte_t *ptep)
{
	/*
	 * The pmd must be loaded with the physical address of the PTE table
	 */
	__pmd_populate(pmdp, __pa(ptep), _PAGE_KERNEL_TABLE);
}

static pte_t *(*pte_offset_fixmap)(pmd_t *dir, unsigned long addr);

static pte_t *pte_offset_early_fixmap(pmd_t *dir, unsigned long addr)
{
	return &bm_pte[pte_index(addr)];
}

static pte_t *pte_offset_late_fixmap(pmd_t *dir, unsigned long addr)
{
	return pte_offset_kernel(dir, addr);
}

void early_fixmap_init(void)
{
	pmd_t *pmd;

	if ((__fix_to_virt(__end_of_early_ioremap_region) >> PMD_SHIFT) !=
					FIXADDR_TOP >> PMD_SHIFT)
		printk("BUILD_BUG_ON(): __end_of_early_ioremap_region\n");

	pmd = fixmap_pmd(FIXADDR_TOP);
	pmd_populate_kernel(&init_mm, pmd, bm_pte);

	pte_offset_fixmap = pte_offset_early_fixmap;
}

/* Establish page table on boot stage */
void __create_page_tables(void)
{
	swapper_pg_dir = memblock_alloc(PG_DIR_SIZE, sizeof(unsigned long));
	/* Clear the swapper page table */
	memset(swapper_pg_dir, 0, PG_DIR_SIZE);
	/* FIXMAP PTE TABLE alloc */
	bm_pte = memblock_alloc((PTRS_PER_PTE + PTE_HWTABLE_PTRS) * 
					sizeof(pte_t), 
					PTE_HWTABLE_OFF + PTE_HWTABLE_SIZE);
	memset(bm_pte, 0, (PTRS_PER_PTE + PTE_HWTABLE_PTRS) * sizeof(pte_t));

	init_mm.pgd = swapper_pg_dir;
}

/* Emulate 32MB VMALLOC Memory */
static void *vmalloc_min =
	(void *)(VMALLOC_END - (240 << 20) - VMALLOC_OFFSET);

phys_addr_t arm_lowmem_limit = 0;

void adjust_lowmem_bounds(void)
{
	phys_addr_t memblock_limit = 0;
	u64 vmalloc_limit;
	struct memblock_region *reg;
	phys_addr_t lowmem_limit = 0;

	/*
	 * Let's use our own (unoptimized) equivalent of __pa() that is
	 * not affected by wrap-arounds when sizeof(phys_addr_t) == 4.
	 * The result is used as the upper bound on physical memory address
	 * and may itself be outsize the valid range for which phys_addr_t
	 * and therefore __pa() is defined.
	 */
	vmalloc_limit = (u64)(unsigned long)vmalloc_min - 
					MMU_PAGE_OFFSET + PHYS_OFFSET;

	for_each_memblock(memory, reg) {
		phys_addr_t block_start = reg->base;
		phys_addr_t block_end = reg->base + reg->size;

		if (reg->base < vmalloc_limit) {
			if (block_end > lowmem_limit)
				/*
				 * Compare as u64 to ensure vmalloc_limit does
				 * not get truncated. block_end should always
				 * fit in phys_addr_t so there should be no
				 * issue with assignment.
				 */
				lowmem_limit = min_t(u64,
							vmalloc_limit,
							block_end);

			/*
			 * Find the first non-pmd-aligned page, and point
			 * memblock_limit at it. This relies on rounding the
			 * limit down to be pmd-aligned, which happens at the
			 * end of this function.
			 *
			 * With this algorithm, the start or end of almost any
			 * bank can be non-pmd-aligned. The only exception is
			 * that the start of the bank 0 must be section-
			 * aligned, since otherwise memory would need to be
			 * allocated when mapping the start of bank 0, which
			 * occurs before any free memory is mapped.
			 */
			if (!memblock_limit) {
				if (!IS_ALIGNED(block_start, PMD_SIZE))
					memblock_limit = block_start;
				else if (!IS_ALIGNED(block_end, PMD_SIZE))
					memblock_limit = lowmem_limit;
			}
		}
	}

	arm_lowmem_limit = lowmem_limit;

	high_memory = (void *)(__mmu_phys_to_virt(arm_lowmem_limit - 1) + 1);

	if (!memblock_limit)
		memblock_limit = arm_lowmem_limit;

	/*
	 * Round the memblock limit down to a pmd size. This
	 * helps to ensure that we will allocate memory from the
	 * last full pmd, which should be mapped.
	 */
	memblock_limit = round_down(memblock_limit, PMD_SIZE);

	if (memblock_end_of_DRAM() > arm_lowmem_limit) {
		phys_addr_t end = memblock_end_of_DRAM();

		memblock_remove(memblock_limit, end - memblock_limit);
	}

	memblock_set_current_limit(memblock_limit);
}

void dup_pgdir(void)
{
	unsigned long addr;

	for (addr = 0; addr < (MAX_VIRTUAL_ADDR & PMD_MASK); 
						addr += PMD_SIZE) {
		if ((unsigned long)pmd_val(*pmd_off_k(addr)))
			printk("ADDR %#lx PMD %#lx: %#lx\n", addr, 
				(unsigned long)pmd_off_k(addr),
				(unsigned long)pmd_val(*pmd_off_k(addr)));
	}
}

static inline void prepare_page_table(void)
{
	unsigned long addr;
	phys_addr_t end;

	/*
	 * Clear out all the mappings below the kernel image
	 */
	for (addr = 0; addr < MODULES_VADDR; addr += PMD_SIZE)
		pmd_clear(pmd_off_k(addr));

	for ( ; addr < PAGE_OFFSET; addr += PMD_SIZE)
		pmd_clear(pmd_off_k(addr));

	/*
	 * Find the end of the first block of lomem.
	 */
	end = memblock.memory.regions[0].base +
			memblock.memory.regions[0].size;
	if (end >= arm_lowmem_limit)
		end = arm_lowmem_limit;

	/*
	 * Clear out all the kernel space mappings, except for the first
	 * memory bank, up to the vmalloc region.
	 */
	for (addr = __mmu_phys_to_virt(end);
			addr < VMALLOC_START; addr += PMD_SIZE)
		pmd_clear(pmd_off_k(addr));

}

static void *early_alloc_aligned(unsigned long sz, unsigned long align)
{
	void *ptr = __va(memblock_phys_alloc(sz, align));
	memset(ptr, 0, sz);
	return ptr;
}

static void *early_alloc(unsigned long sz)
{
	return early_alloc_aligned(sz, sz);
}

static void __map_init_section(pmd_t *pmd, unsigned long addr,
			unsigned long end, phys_addr_t phys,
			const struct mem_type *type, bool ng)
{
	pmd_t *p = pmd;

	/*
	 * In classic MMU format, puds and pmds are folded in to
	 * the pgds. pmd_offset gives the PGD entry. PGDs refer to a
	 * group of L1 entries making up one logical pointer to
	 * and L2 table (2MB), where as PMDs refer to the individual
	 * L1 entries (1MB). Hence increment to get the correct
	 * offset for odd 1MB sections
	 */
	if (addr & SECTION_SIZE)
		pmd++;

	do {
		*pmd = __pmd(phys | type->prot_sect | (ng ? PMD_SECT_nG : 0));
		phys += SECTION_SIZE;
	} while (pmd++, addr += SECTION_SIZE, addr != end);
}

static pte_t *arm_pte_alloc(pmd_t *pmd, unsigned long addr,
			unsigned long prot,
			void *(*alloc)(unsigned long sz))
{
	if (pmd_none(*pmd)) {
		pte_t *pte = alloc(PTE_HWTABLE_OFF + PTE_HWTABLE_SIZE);
		__pmd_populate(pmd, __pa(pte), prot);
	}
	if (pmd_bad(*pmd));
	return pte_offset_kernel(pmd, addr);
}

static void alloc_init_pte(pmd_t *pmd, unsigned long addr,
			unsigned long end, unsigned long pfn,
			const struct mem_type *type,
			void *(*alloc)(unsigned long sz),
			bool ng)
{
	pte_t *pte = arm_pte_alloc(pmd, addr, type->prot_l1, alloc);
	do {
		set_pte_ext(pte, pfn_pte(pfn, __pgprot(type->prot_pte)),
				ng ? PTE_EXT_NG : 0);
		pfn++;
	} while (pte++, addr += PAGE_SIZE, addr != end);
}

static void alloc_init_pmd(pud_t *pud, unsigned long addr,
			unsigned long end, phys_addr_t phys,
			const struct mem_type *type,
			void *(*alloc)(unsigned long sz), bool ng)
{
	pmd_t *pmd = pmd_offset(pud, addr);
	unsigned long next;

	do {
		/*
		 * With LPAE, we must loop over to map
		 * all the pmds for the given range.
		 */
		next = pmd_addr_end(addr, end);

		/*
		 * Try a section mapping - addr, next and phys must all be
		 * aligned to a section boundary.
		 */
		if (type->prot_sect &&
				((addr | next | phys) & ~SECTION_MASK) == 0) {
			__map_init_section(pmd, addr, next, phys, type, ng);
		} else {
			alloc_init_pte(pmd, addr, next,
				__phys_to_pfn(phys), type, alloc, ng);
		}

		phys += next - addr;
	} while (pmd++, addr = next, addr != end);
}


static void alloc_init_pud(pgd_t *pgd, unsigned long addr,
			unsigned long end, phys_addr_t phys,
			const struct mem_type *type,
			void *(*alloc)(unsigned long sz),
			bool ng)
{
	pud_t *pud = pud_offset(pgd, addr);
	unsigned long next;

	do {
		next = pud_addr_end(addr, end);
		
		alloc_init_pmd(pud, addr, next, phys, type, alloc, ng);
		phys += next - addr;
	} while (pud++, addr = next, addr != end);
}

static void __create_mapping(struct mm_struct *mm, struct map_desc *md,
			void *(*alloc)(unsigned long sz),
			bool ng)
{
	unsigned long addr, length, end;
	phys_addr_t phys;
	const struct mem_type *type;
	pgd_t *pgd;

	type = &mem_types[md->type];

	addr = md->virtual & PAGE_MASK;
	phys = __pfn_to_phys(md->pfn);
	length = PAGE_ALIGN(md->length + (md->virtual & ~PAGE_MASK));

	if (type->prot_l1 == 0 && ((addr | phys | length) & ~SECTION_MASK)) {
		printk("BUG: map for %#lx at %#lx can not be mapped using "
			"pages, ignoring.\n",
				(unsigned long)__pfn_to_phys(md->pfn), addr);
		return;
	}

	pgd = pgd_offset(mm, addr);
	end = addr + length;
	do {
		unsigned long next = pgd_addr_end(addr, end);

		alloc_init_pud(pgd, addr, next, phys, type, alloc, ng);

		phys += next - addr;
		addr = next;
	} while (pgd++, addr != end);
}

/*
 * Create the page directory entries and any necessary
 * page table for the mapping specified by 'md'. We
 * are able to cope here with varying sizes and address
 * offsets, and we take full advantage of sections and
 * supersections.
 */
static void create_mapping(struct map_desc *md)
{
	if (md->virtual != vectors_base() && md->virtual < TASK_SIZE) {
		printk("BUG: not creating mapping for %#lx at %#lx in "
						"user region\n",
		(unsigned long)__pfn_to_phys((u64)md->pfn), md->virtual);
		return;
	}

	if ((md->type == MT_DEVICE || md->type == MT_ROM) &&
		md->virtual >= PAGE_OFFSET && md->virtual < FIXADDR_START &&
		(md->virtual < VMALLOC_START || md->virtual >= VMALLOC_END)) {
		printk("BUG: mapping for %#lx at %#lx out of vmalloc space\n",
		(unsigned long)__pfn_to_phys((u64)md->pfn), md->virtual);
	}

	__create_mapping(&init_mm, md, early_alloc, false);
}

static void map_lowmem(void)
{
	struct memblock_region *reg;
	phys_addr_t kernel_x_start = round_down(__pa(KERNEL_START), 
							SECTION_SIZE);
	phys_addr_t kernel_x_end = round_down(__pa(KERNEL_END), SECTION_SIZE);

	/* Map all the lowmem memory banks */
	for_each_memblock(memory, reg) {
		phys_addr_t start = reg->base;
		phys_addr_t end = start + reg->size;
		struct map_desc map;

		if (end > arm_lowmem_limit)
			end = arm_lowmem_limit;
		if (start >= end)
			break;

		if (end < kernel_x_start) {
			map.pfn = __phys_to_pfn(start);
			map.virtual = __mmu_phys_to_virt(start);
			map.length = end - start;
			map.type = MT_MEMORY_RWX;

			//create_mapping(&map);
		} else if (start >= kernel_x_end) {
			map.pfn = __phys_to_pfn(start);
			map.virtual = __mmu_phys_to_virt(start);
			map.length = end - start;
			map.type = MT_MEMORY_RW;

			//create_mapping(&map);
		} else {
			/* This better cover the entire kernel */
			if (start < kernel_x_start) {
				map.pfn = __phys_to_pfn(start);
				map.virtual = __mmu_phys_to_virt(start);
				map.length = kernel_x_start - start;
				map.type = MT_MEMORY_RW;

				create_mapping(&map);
			}

			map.pfn = __phys_to_pfn(kernel_x_start);
			map.virtual = __mmu_phys_to_virt(kernel_x_start);
			map.length = kernel_x_end - kernel_x_start;
			map.type = MT_MEMORY_RWX;

			create_mapping(&map);

			if (kernel_x_end < end) {
				map.pfn = __phys_to_pfn(kernel_x_end);
				map.virtual = __mmu_phys_to_virt(kernel_x_end);
				map.length = end - kernel_x_end;
				map.type = MT_MEMORY_RW;

				create_mapping(&map);
			}
		}
	}
}

static void early_fixmap_shutdown(void)
{
	int i;
	unsigned long va = fix_to_virt(__end_of_permanent_fixed_addresses - 1);

	pte_offset_fixmap = pte_offset_late_fixmap;
	pmd_clear(fixmap_pmd(va));

	for (i = 0; i < __end_of_permanent_fixed_addresses; i++) {
		pte_t *pte;
		struct map_desc map;

		map.virtual = fix_to_virt(i);
		pte = pte_offset_early_fixmap(pmd_off_k(map.virtual),
								map.virtual);
		/* Only  i/o device mapping are supported ATM */
		if (pte_none(*pte) ||
		   (pte_val(*pte) & L_PTE_MT_MASK) != L_PTE_MT_DEV_SHARED)
			continue;

		map.pfn = pte_pfn(*pte);
		map.type = MT_DEVICE;
		map.length = PAGE_SIZE;

		create_mapping(&map);
	}
}

/*
 * Set up the device mappings. Since we clear out the page tables for all
 * mappings above VMALLOC_START, except early fixmap, we might remove debug
 * device mappings. This means earlycon can be used to debug this function
 * Any other function or debugging method which may touch any device _will_
 * crash the kernel.
 */
static void devicemaps_init(void)
{
	struct map_desc map;
	unsigned long addr;
	void *vectors;

	/*
	 * Allocate the vector page early.
	 */
	vectors = early_alloc(PAGE_SIZE * 2);

	/*
	 * Clear page table except top pmd used by early fixmaps
	 */
	for (addr = VMALLOC_START; addr < (FIXADDR_TOP & PMD_MASK);
					addr += PMD_SIZE)
		pmd_clear(pmd_off_k(addr));

	/*
	 * Create a mapping for the machine vectors at the high-vectors
	 * location (0xffff0000). If we aren't using high-vectors, also
	 * create a mapping at the low-vectors virtual address.
	 */
	map.pfn = __phys_to_pfn(virt_to_phys(vectors));
	map.virtual = 0xffff0000;
	map.length = PAGE_SIZE;
	map.type = MT_HIGH_VECTORS;

	create_mapping(&map);

}

/*
 * paging_init() sets up the page tables, initialises the zone memory
 * maps, and sets up the zero page, bad page and bad page tables.
 */
void paging_init(void)
{
	void *zero_page;

	prepare_page_table();
	map_lowmem();
	memblock_set_current_limit(arm_lowmem_limit);
	early_fixmap_shutdown();
	devicemaps_init();
}

void page_table_init(void)
{
	__create_page_tables();
	early_fixmap_init();
	adjust_lowmem_bounds();
	paging_init();
}
