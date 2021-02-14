/*
 * IO Memory Mapping With 32-Bit Paging
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
/* page table */
#include <asm/pgalloc.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#define DEV_NAME		"BiscuitOS"
/* Platform device */
static struct platform_device *BiscuitOS_pdev;
/* BiscuitOS IO */
#define BISCUITOS_IO_BASE	0xfec10000
#define BISCUITOS_IO_SIZE	PAGE_SIZE

static int ioremap_pte_range(pmd_t *pde, unsigned long addr,
		unsigned long end, phys_addr_t phys_addr, pgprot_t prot)
{
	pte_t *pte;
	u64 pfn;

	pfn = phys_addr >> PAGE_SHIFT;
	pte = pte_alloc_kernel(pde, addr);
	if (!pte)
		return -ENOMEM;

	do {
		/* BiscuitOS Emulate IO Memory */
		struct page *page = alloc_page(GFP_KERNEL | __GFP_HIGHMEM);
		pfn = page_to_pfn(page);

		BUG_ON(!pte_none(*pte));
		set_pte_at(&init_mm, addr, pte, pfn_pte(pfn, prot));
		pfn++;
	} while (pte++, addr += PAGE_SIZE, addr != end);
	return 0;
}

static int BiscuitOS_ioremap_page_range(unsigned long addr,
		unsigned long end, phys_addr_t phys_addr, pgprot_t prot)
{
	pgd_t *pgd;
	unsigned long start, next;

	might_sleep();
	BUG_ON(addr >= end);

	start = addr;
	pgd = pgd_offset_k(addr);
	do {
		pmd_t *pde;

		/* 32-Bit Paging only contains PDE and PTE */
		pde = (pmd_t *)pgd;

		next = pgd_addr_end(addr, end);
		if (ioremap_pte_range(pde, addr, next, phys_addr, prot))
			return -ENOMEM;
	} while (pgd++, phys_addr += (next - addr), addr = next, addr != end);

	flush_cache_vmap(start, end);
	return 0;
}

static void __iomem *BiscuitOS_ioremap(resource_size_t phys_addr, 
							unsigned long size)
{
	struct vm_struct *area;
	unsigned long vaddr;
	pgprot_t prot;

	/* Mapping have to be page-aligned */
	phys_addr &= PHYSICAL_PAGE_MASK;

	/*
	 * If the page being mapped is in memory and SEV is active then
	 * make sure the memory encryption attribute is enabled in the
	 * resulting mapping.
	 */
	prot = PAGE_KERNEL_IO;
	prot = __pgprot(pgprot_val(prot) |
			cachemode2protval(_PAGE_CACHE_MODE_UC));
	
	area = get_vm_area_caller(size, VM_IOREMAP, 
					__builtin_return_address(0));
	area->phys_addr = phys_addr;
	vaddr = (unsigned long)area->addr;

	BiscuitOS_ioremap_page_range(vaddr, vaddr + size, phys_addr, prot);

	return (void __iomem *)vaddr;
}

static void vunmap_pte_range(pmd_t *pde, unsigned long addr, unsigned long end)
{
	pte_t *pte;

	pte = pte_offset_kernel(pde, addr);
	do {
		pte_t ptent;
		/* BiscuitOS Emulate IO Memory */
		struct page *page = pte_page(*pte);
		__free_page(page);

		ptent = ptep_get_and_clear(&init_mm, addr, pte);
		WARN_ON(!pte_none(ptent) && !pte_present(ptent));
	} while (pte++, addr += PAGE_SIZE, addr != end);
}

static void vunmap_page_range(unsigned long addr, unsigned long end)
{
	pgd_t *pgd;
	unsigned long next;

	BUG_ON(addr >= end);
	pgd = pgd_offset_k(addr);

	do {
		pmd_t *pde;

		/* 32-Bit Paging only contains PDE and PTE */
		pde = (pmd_t *)pgd;
		next = pgd_addr_end(addr, end);

		vunmap_pte_range(pde, addr, next);
	} while (pgd++, addr = next, addr != end);
}

static void BiscuitOS_remove_vm_area(const void *addr)
{
	unsigned long start = (unsigned long)addr;
	unsigned long end   = start + BISCUITOS_IO_SIZE;

	flush_cache_vunmap(start, end);
	vunmap_page_range(start, end);

	flush_tlb_kernel_range(start, end);
}

static void BiscuitOS_iounmap(volatile void __iomem *addr)
{
	struct vm_struct *p;

	addr = (volatile void __iomem *)
			(PAGE_MASK & (unsigned long __force)addr);

	p = find_vm_area((void __force *)addr);
	if (!p) {
		printk(KERN_ERR "iounmap: bad adderss %p\n", addr);
		dump_stack();
		return;
	}

	BiscuitOS_remove_vm_area((void __force *)addr);
	kfree(p);
}

/* Probe: (DDL) Initialize Device */
static int BiscuitOS_probe(struct platform_device *pdev)
{
	struct resource *r;
	unsigned long __iomem *addr;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	addr = BiscuitOS_ioremap(r->start, r->end - r->start);

	/* Use */
	*addr = 88520;

	printk("\n\n\n\n******************BiscuitOS*****************\n");
	printk("=> %#lx: %ld\n", (unsigned long)addr, *addr);
	printk("*********************************************\n\n\n\n");

	/* Release */
	BiscuitOS_iounmap(addr);

	/* Trigger page fault if access addr */
	//*addr = 88520;

	return 0;
}

static struct resource BiscuitOS_resources[] = {
	[0] = { /* Memory Region */
		.name = "BiscuitOS IO Memory",
		.start = BISCUITOS_IO_BASE,
		.end   = BISCUITOS_IO_BASE + BISCUITOS_IO_SIZE,
		.flags = IORESOURCE_MEM,
	}
};

/* Platform Driver Information */
static struct platform_driver BiscuitOS_driver = {
	.probe    = BiscuitOS_probe,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	int ret;

	/* Register platform driver */
	ret = platform_driver_register(&BiscuitOS_driver);
	if (ret) {
		printk("Unable register Platform driver.\n");
		return -EBUSY;
	}

	/* Register platform device */
	BiscuitOS_pdev = platform_device_register_simple(DEV_NAME, 
					-1, BiscuitOS_resources, 1);
	if (!BiscuitOS_pdev) {
		printk("Unable register Platform device.\n");
		return -EBUSY;
	}

	return 0;
}
device_initcall(BiscuitOS_init);
