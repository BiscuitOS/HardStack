#ifndef _BISCUITOS_PERCPU_H
#define _BISCUITOS_PERCPU_H

#include "linux/list.h"
#include "linux/bitmap.h"

/* CPUs */
#ifdef CONFIG_SMP
#define NR_CPUS		CONFIG_NR_CPUS
#define nr_cpu_ids	NR_CPUS
#else
#define NR_CPUS		1U
#define nr_cpu_ids	1U
#endif

/* enough to cover all DEFINE_PER_CPUs in modules */
#define PERCPU_MODULE_RESERVE		(8 << 10)
#define PERCPU_DYNAMIC_RESERVE		(20 << 10)
#define MAX_DMA_ADDRESS			0xffffffffUL

/* Emualte percpu */
#define __percpu

#define num_possible_cpus()		(NR_CPUS)

/*
 * Percpu allocator can serve perfcpu allocations before slab is
 * initialized which allows slab to depend on the percpu allocator.
 * The following two parameters decide how much resource to
 * preallocate for this.  Keep PERCPU_DYNAMIC_RESERVE equal to or
 * larger then PERCPU_DYNAMIC_EARLY_SIZE.
 */
#define PERCPU_DYNAMIC_EARLY_SLOTS	128
#define PERCPU_DYNAMIC_EARLY_SIZE	(12 << 10)

/* minimum unit size, also is the maximum supported allocation size */
#define PCPU_MIN_UNIT_SIZE		PFN_ALIGN(32 << 10)

/* minimum allocation size and shift in bytes */
#define PCPU_MIN_ALLOC_SHIFT		2
#define PCPU_MIN_ALLOC_SIZE		(1 << PCPU_MIN_ALLOC_SHIFT)

/* number of bits per page, unsed to trigger a scan if blocks are > PAGE_SIZE */
#define PCPU_BITS_PER_PAGE		(PAGE_SIZE >> PCPU_MIN_ALLOC_SHIFT)

/*
 * This determines the size of each metadata block.  There are several subtle
 * constraints around this constant.  The reserved region must be a multiple of
 * PCPU_BITMAP_BLOCK_SIZE.  Additionally, PCPU_BITMAP_BLOCK_SIZE must be a
 * multiple of PAGE_SIZE or PAGE_SIZE must be a multiple of
 * PCPU_BITMAP_BLOCK_SIZE to align with the populated page map. The unit_size
 * also has to be a multiple of PCPU_BITMAP_BLOCK_SIZE to ensure full blocks.
 */
#define PCPU_BITMAP_BLOCK_SIZE		PAGE_SIZE
#define PCPU_BITMAP_BLOCK_BITS		(PCPU_BITMAP_BLOCK_SIZE >> \
					 PCPU_MIN_ALLOC_SHIFT)

/* the slots are sorted by free bytes left, 1-31 bytes share the same slot */
#define PCPU_SLOT_BASE_SHIFT		5

#define PCPU_EMPTY_POP_PAGES_LOW	2
#define PCPU_EMPTY_POP_PAGES_HIGH	4

struct pcpu_group_info {
	int			nr_units;	/* aligned # of units */
	unsigned long		base_offset;	/* base address offset */
	unsigned int		*cpu_map;	/* unit->cpu map, empty
						 * entries contain NR_CPUS */
};

struct pcpu_alloc_info {
	size_t			static_size;
	size_t			reserved_size;
	size_t			dyn_size;
	size_t			unit_size;
	size_t			atom_size;
	size_t			alloc_size;
	size_t			__ai_size;	/* internal, don't use */
	size_t			nr_groups;	/* 0 if grouping unnecessary */
	struct pcpu_group_info	groups[];
};

/*
 * pcpu_block_md is the metadata block struct.
 * Each chunk's bitmap is split into a number of full blocks.
 * All units are in terms of bits.
 */
struct pcpu_block_md {
	int 			contig_hint;	/* contig hint for block */
	int			contig_hint_start; /* block relative starting
						 position of the contig hit */
	int			left_free;	/* size of free space along 
						 * the left side of the block */
	int			right_free;	/* size of free space along
					* the right side of the block */
	int			first_free; /* block position of first free */
};

struct pcpu_chunk {
	int			nr_alloc;	/* # of allocations */
	size_t			max_alloc_size;	/* largest allocation size */
	struct list_head	list;		/* linked to pcpu_slot lists */
	int			free_bytes;	/* free bytes in the chunk */
	int 			contig_bits;	/* max contiguous size hint */
	int			contig_bits_start; /* contig_bits starting
						    * offset */
	void			*base_addr;	/* base address of this chunk */

	unsigned long		*alloc_map;	/* allocation map */
	unsigned long		*bound_map;	/* boundary map */
	struct pcpu_block_md	*md_blocks;	/* metadata blocks */

	void			*data;		/* chunk data */
	int			first_bit;	/* no free below this */
	bool			immutable;	/* no [de]population allowed */
	int			start_offset;	/* the overlap with the previous
						 * region to have a page aligned
						 * base_addr */
	int			end_offset;	/* additional area required to
						 * have the region end page
						 * aligned */

	int			nr_pages;	/* # of pages served by this chunk */
	int			nr_populated;	/* # of populated pages */
	int			nr_empty_pop_pages; /* # of empty populated pages */
	unsigned long		populated[];	/* populated bitmap */
};

struct percpu_stats {
	u64 nr_alloc;           /* lifetime # of allocations */
	u64 nr_dealloc;         /* lifetime # of deallocations */
	u64 nr_cur_alloc;       /* current # of allocations */
	u64 nr_max_alloc;       /* max # of live allocations */
	u32 nr_chunks;          /* current # of live chunks */
	u32 nr_max_chunks;      /* max # of live chunks */
	size_t min_alloc_size;  /* min allocaiton size */
	size_t max_alloc_size;  /* max allocation size */
};

typedef void (*pcpu_fc_free_fn_t)(void *ptr, size_t size);
typedef void *(*pcpu_fc_alloc_fn_t)(unsigned int cpu, size_t size,
							size_t align);
typedef int (pcpu_fc_cpu_distance_fn_t)(unsigned int from, unsigned int to);

#define cpu_possible_mask		0xff
#define for_each_cpu(cpu, mask)		\
	for ((cpu) = 0; (cpu) < NR_CPUS; (cpu)++, (void)mask)
#define for_each_possible_cpu(cpu)	for_each_cpu((cpu), cpu_possible_mask)

static inline int pcpu_chunk_nr_blocks(struct pcpu_chunk *chunk)
{
	return chunk->nr_pages * PAGE_SIZE / PCPU_BITMAP_BLOCK_SIZE;
}

static inline int pcpu_nr_pages_to_map_bits(int pages)
{
	return pages * PAGE_SIZE / PCPU_MIN_ALLOC_SIZE;
}

/**
 * pcpu_chunk_map_bits - helper to convert nr_pages to size of bitmap
 */
static inline int pcpu_chunk_map_bits(struct pcpu_chunk *chunk)
{
	return pcpu_nr_pages_to_map_bits(chunk->nr_pages);
}

extern void __percpu *__alloc_percpu(size_t size, size_t align);

#define alloc_percpu(type)					\
	(typeof(type) __percpu *)__alloc_percpu(sizeof(type),	\
					__alignof__(type))

extern struct percpu_stats pcpu_stats;

static inline void pcpu_stats_area_alloc(struct pcpu_chunk *chunk, size_t size)
{
	pcpu_stats.nr_alloc++;
	pcpu_stats.nr_cur_alloc++;
	pcpu_stats.nr_max_alloc =
		max(pcpu_stats.nr_max_alloc, pcpu_stats.nr_cur_alloc);
	pcpu_stats.min_alloc_size =
		min(pcpu_stats.min_alloc_size, size);
	pcpu_stats.max_alloc_size =
		max(pcpu_stats.max_alloc_size, size);

	chunk->nr_alloc++;
	chunk->max_alloc_size = max(chunk->max_alloc_size, size);
}

static inline void pcpu_stats_area_dealloc(struct pcpu_chunk *chunk)
{
	pcpu_stats.nr_dealloc++;
	pcpu_stats.nr_cur_alloc--;

	chunk->nr_alloc--;
}

static int pcpu_populate_chunk(struct pcpu_chunk *chunk,
			int page_start, int page_end, gfp_t gfp)
{
	return 0;
}

#define __verify_pcpu_ptr(ptr)						\
do {									\
	const void __percpu *__vpp_verify = (typeof((ptr) + 0))NULL;	\
	(void)__vpp_verify;						\
} while (0)

#ifdef CONFIG_SMP

#define RELOC_HIDE(ptr, off)						\
({									\
	unsigned long __ptr;						\
	__ptr = (unsigned long)(ptr);					\
	(typeof(ptr)) (__ptr + (off));					\
})

extern unsigned long __per_cpu_offset[NR_CPUS];

#define per_cpu_offset(x)	(__per_cpu_offset[x])

/*      
 * Add an offset to a pointer but keep the pointer as-is.  Use RELOC_HIDE()
 * to prevent the compiler from making incorrect assumptions about the
 * pointer value.  The weird cast keeps both GCC and sparse happy.
 */ 
#define SHIFT_PERCPU_PTR(__p, __offset)					\
	RELOC_HIDE((typeof(*(__p)) *)(__p), (__offset))

#define per_cpu_ptr(ptr, cpu)						\
({									\
	__verify_pcpu_ptr(ptr);						\
	SHIFT_PERCPU_PTR((ptr), per_cpu_offset((cpu)));			\
})

#define raw_cpu_ptr(ptr)						\
({									\
	__verify_pcpu_ptr(ptr);						\
	arch_raw_cpu_ptr(ptr);						\
})

#define this_cpu_ptr(ptr)						\
({									\
	__verify_pcpu_ptr(ptr);						\
	SHIFT_PERCPU_PTR(ptr, my_cpu_offset);				\
})

#else /* CONFIG_UP */

#define VERIFY_PERCPU_PTR(__p)						\
({									\
	__verify_pcpu_ptr(__p);					\
	(typeof(*(__p)) *)(__p);					\
})

#define per_cpu_ptr(ptr, cpu)	({ (void)(cpu); VERIFY_PERCPU_PTR(ptr); })
#define raw_cpu_ptr(ptr)	per_cpu_ptr(ptr, 0)
#define this_cpu_ptr(ptr)	raw_cpu_ptr(ptr)
#endif

extern void *pcpu_base_addr;

extern void setup_per_cpu_areas(void);
extern void free_percpu(void __percpu *ptr);
#endif
