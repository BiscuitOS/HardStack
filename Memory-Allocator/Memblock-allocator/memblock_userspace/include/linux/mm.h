#ifndef _BISCUITOS_MM_H
#define _BISCUITOS_MM_H

#include "linux/biscuitos.h"

#define PAGE_SHIFT	12
#define PAGE_SIZE	(1 << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

#define PAGE_ALIGN(addr)	ALIGN(addr, PAGE_SIZE)
#define PAGE_ALIGNED(addr)	IS_ALIGNED((unsigned long)(addr), PAGE_SIZE)

/* Phys and virutal */
extern const char *memory;

#define PAGE_OFFSET	((unsigned long)memory)

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

#define PHYS_OFFSET	__pa(PAGE_OFFSET)

#endif
