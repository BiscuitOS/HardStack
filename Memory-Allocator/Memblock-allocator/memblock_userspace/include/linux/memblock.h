#ifndef _BISCUITOS_MEMBLOCK_H
#define _BISCUITOS_MEMBLOCK_H

#include "linux/mm.h"

/**
 * enum memblock_flags - definition of memory region attributes
 * @MEMBLOCK_NONE: no special request
 * @MEMBLOCK_HOTPLUG: hotpluggable region
 * @MEMBLOCK_MIRROR: mirrored region
 * @MEMBLOCK_NOMAP: don't add to kernel direct mapping
 */
enum memblock_flags {
	MEMBLOCK_NONE		= 0x0, /* No special request */
	MEMBLOCK_HOTPLUG	= 0x1, /* hotpluggable region */
	MEMBLOCK_MIRROR		= 0x2, /* mirrored region */
	MEMBLOCK_NOMAP		= 0x4, /* don't add to kernel direct mapping */
};

/**
 * struct memblock_region - represents a memory region
 * @base: physical address of the region
 * @size: size of the region
 * @flags: memory region attributes
 */
struct memblock_region {
	phys_addr_t base;
	phys_addr_t size;
	enum memblock_flags flags;
};

/**
 * struct memblock_type - collection of memory regions of certain type
 * @cnt: number of regions
 * @max: size of the allocated array
 * @total_size: size of all regions
 * @regions: array of regions
 * @name: the memory type symbolic name
 */
struct memblock_type {
	unsigned long cnt;
	unsigned long max;
	phys_addr_t total_size;
	struct memblock_region *regions;
	char *name;
};

/**
 * struct memblock - memblock allocator metadata
 * @bottom_up: is bottom up direction?
 * @current_limit: physical address of the current allocation limit
 * @memory: usabe memory regions
 * @reserved: reserved memory regions
 * @physmem: all physical memory
 */
struct memblock {
	bool bottom_up; /* is bottom up direction? */
	phys_addr_t current_limit;
	struct memblock_type memory;
	struct memblock_type reserved;
};

#define MEMBLOCK_ALLOC_ANYWHERE	(~(phys_addr_t)0)
#define MEMBLOCK_ALLOC_ACCESSIBLE	0
#define MEMBLOCK_ALLOC_KASAN		1
#define MEMBLOCK_LOW_LIMIT		0

#define INIT_MEMBLOCK_REGIONS		32
#define INIT_MEMBLOCK_RESERVED_REGIONS	INIT_MEMBLOCK_REGIONS

extern struct memblock memblock;

static inline void memblock_set_region_node(struct memblock_region *r, int nid)
{
}

static inline int memblock_get_region_node(const struct memblock_region *r)
{
	return 0;
}

#define for_each_memblock_type(i, memblock_type, rgn)			\
	for (i = 0, rgn = &memblock_type->regions[0];			\
		i < memblock_type->cnt;					\
		i++, rgn = &memblock_type->regions[i])

#define for_each_mem_range(i, type_a, type_b, nid, flags,		\
				p_start, p_end, p_nid)			\
	for (i = 0, __next_mem_range(&i, nid, flags, type_a, type_b,	\
				p_start, p_end, p_nid);			\
		i != (u64)ULLONG_MAX;					\
		__next_mem_range(&i, nid, flags, type_a, type_b,	\
				p_start, p_end, p_nid))

#define for_each_mem_range_rev(i, type_a, type_b, nid, flags,		\
				p_start, p_end, p_nid)			\
	for (i = (u64)ULLONG_MAX,					\
		__next_mem_range_rev(&i, nid, flags, type_a, type_b,	\
					p_start, p_end, p_nid);		\
		i != (u64)ULLONG_MAX;					\
		__next_mem_range_rev(&i, nid, flags, type_a, type_b,	\
					p_start, p_end, p_nid))

#define for_each_free_mem_range(i, nid, flags, p_start, p_end, p_nid)	\
	for_each_mem_range(i, &memblock.memory, &memblock.reserved,	\
			nid, flags, p_start, p_end, p_nid)

#define for_each_free_mem_range_reverse(i, nid, flags, p_start, p_end,	\
						p_nid)			\
	for_each_mem_range_rev(i, &memblock.memory, &memblock.reserved,	\
				nid, flags, p_start, p_end, p_nid)

#define for_each_memblock(memblock_type, region)			\
	for (region = memblock.memblock_type.regions;			\
		region < (memblock.memblock_type.regions + 		\
				memblock.memblock_type.cnt);		\
		region++)

/*
 * Check if the allocation is bottom-up or not.
 * if this is true, that said, memblock will allocate memory
 * in bottom-up direction.
 */
static inline bool memblock_bottom_up(void)
{
	return memblock.bottom_up;
}

static inline void memblock_set_bottom_up(bool direct)
{
	memblock.bottom_up = direct;
}

static inline bool movable_node_is_enabled(void)
{
	return false;
}

static inline bool memblock_is_hotpluggable(struct memblock_region *m)
{
	return m->flags & MEMBLOCK_HOTPLUG;
}

static inline bool memblock_is_mirror(struct memblock_region *m)
{
	return m->flags & MEMBLOCK_MIRROR;
}

static inline bool memblock_is_nomap(struct memblock_region *m)
{
	return m->flags & MEMBLOCK_NOMAP;
}

extern void *memblock_alloc_try_nid(phys_addr_t size, phys_addr_t align,
		phys_addr_t min_addr, phys_addr_t max_addr, int nid);

static inline void *memblock_alloc(phys_addr_t size, phys_addr_t align)
{
	memblock_alloc_try_nid(size, align, MEMBLOCK_LOW_LIMIT,
				MEMBLOCK_ALLOC_ACCESSIBLE, NUMA_NO_NODE);
}

extern void memory_init(void);
extern void memory_exit(void);
extern int memblock_free(phys_addr_t base, phys_addr_t size);
extern phys_addr_t memblock_phys_alloc(phys_addr_t size, phys_addr_t align);
extern int memblock_reserve(phys_addr_t base, phys_addr_t size);
static int memblock_double_array(struct memblock_type *type,
					phys_addr_t new_area_start,
					phys_addr_t new_area_size);
int memblock_add_range(struct memblock_type *type,
				phys_addr_t base, phys_addr_t size,
				int nid, enum memblock_flags flags);

#endif
