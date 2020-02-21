/*
 * PERCPU Memory Allocator
 *
 * (C) 2020.02.19 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "linux/biscuitos.h"
#include "linux/memblock.h"
#include "linux/percpu.h"
#include "linux/getorder.h"

static unsigned long __per_cpu_start;
static unsigned long __per_cpu_end;
static unsigned long __per_cpu_load;

static int pcpu_nr_units;
static int pcpu_unit_pages;
static int pcpu_unit_size;
static int pcpu_atom_size;
static size_t pcpu_chunk_struct_size;
int pcpu_nr_slots;
const unsigned long *pcpu_unit_offsets;
static const int *pcpu_unit_map;
static const size_t *pcpu_group_sizes;
static const unsigned long *pcpu_group_offsets;
static int pcpu_nr_groups;

/* cpus with the lowest and highest unit address */
static unsigned int pcpu_low_unit_cpu;
static unsigned int pcpu_high_unit_cpu;

struct percpu_stats pcpu_stats;
struct pcpu_alloc_info pcpu_stats_ai;

struct percpu_stats pcpu_stats;

/* chunk list slots */
struct list_head *pcpu_slot;

/*
 * Optional reserved chunk.  This chunk reserves part of the first
 * chunk and serves it for reserved allocations.  When the reserved
 * region doesn't exist, the following variable is NULL.
 */
struct pcpu_chunk *pcpu_reserved_chunk;

/*
 * The number of empty populated pages, protected by pcpu_lock.  The
 * reserved chunk doesn't contribute to the count.
 */
int pcpu_nr_empty_pop_pages;

/*
 * The first chunk which always exists.  Note that unlike other
 * chunks, this one can be allocated and mapped in several different
 * ways and thus often doesn't live in the vmalloc area.
 */
struct pcpu_chunk *pcpu_first_chunk;

/*
 * The number of populated pages in use by the allocator, protected by
 * pcpu_lock.  This number is kept per a unit per chunk (i.e. when a page gets
 * allocated/deallocated, it is allocated/deallocated in all units of a chunk
 * and increments/decrements this count by 1).
 */
static unsigned long pcpu_nr_populated;

/* the address of the first chunk which starts with the kernel static area */
void *pcpu_base_addr;

#ifdef CONFIG_SMP
#define __addr_to_pcpu_ptr(addr)					\
	(void __percpu *)((unsigned long)(addr) -			\
			  (unsigned long)pcpu_base_addr +		\
			  (unsigned long)__per_cpu_start)

#define __pcpu_ptr_to_addr(ptr)						\
	(void __percpu *)((unsigned long)(ptr) +			\
			  (unsigned long)pcpu_base_addr -		\
			  (unsigned long)__per_cpu_start)

#else /* CONFIG_UP */
#define __addr_to_pcpu_ptr(addr)	(void __percpu *)(addr)
#define __pcpu_ptr_to_addr(ptr)		(void *)(ptr)
#endif

/**
 * pcpu_alloc_alloc_info - allocate percpu allocation info
 */
struct pcpu_alloc_info *pcpu_alloc_alloc_info(int nr_groups, int nr_units)
{
	struct pcpu_alloc_info *ai;
	size_t base_size, ai_size;
	void *ptr;
	int unit;

	base_size = ALIGN(sizeof(*ai) + nr_groups * sizeof(ai->groups[0]),
			__alignof__(ai->groups[0].cpu_map[0]));
	ai_size = base_size + nr_units * sizeof(ai->groups[0].cpu_map[0]);

	ptr = memblock_alloc_nopanic(PFN_ALIGN(ai_size), PAGE_SIZE);
	if (!ptr)
		return NULL;
	ai = ptr;
	ptr += base_size;

	ai->groups[0].cpu_map = ptr;

	for (unit = 0; unit < nr_units; unit++)
		ai->groups[0].cpu_map[unit] = NR_CPUS;

	ai->nr_groups = nr_groups;
	ai->__ai_size = PFN_ALIGN(ai_size);

	return ai;
}

/**
 * pcpu_build_alloc_info - build alloc_info considering distances between CPUs
 */
static struct pcpu_alloc_info *pcpu_build_alloc_info(
				size_t reserved_size, size_t dyn_size,
				size_t atom_size,
				pcpu_fc_cpu_distance_fn_t cpu_distance_fn)
{
	static int group_map[NR_CPUS];
	static int group_cnt[NR_CPUS];
	const size_t static_size = __per_cpu_end - __per_cpu_start;
	int nr_groups = 1, nr_units = 0;
	size_t size_sum, min_unit_size, alloc_size;
	int upa, max_upa;
	int best_upa; /* units_per_alloc */
	int last_allocs, group, unit;
	unsigned int cpu, tcpu;
	struct pcpu_alloc_info *ai;
	unsigned int *cpu_map;

	/* this function may be called multiple times */
	memset(group_map, 0, sizeof(group_map));
	memset(group_cnt, 0, sizeof(group_cnt));

	/* calculate size_sum and ensure dyn_size is enough for early alloc */
	size_sum = PFN_ALIGN(static_size + reserved_size +
			max_t(size_t, dyn_size, PERCPU_DYNAMIC_EARLY_SIZE));
	dyn_size = size_sum - static_size - reserved_size;

	/*
	 * Determine min_unit_size, alloc_size and max_upa such that
	 * alloc_size is multiple of atom_size and is the smallest
	 * which can accommodate 4k aligned segments which are equal to
	 * or larger than min_unit_size.
	 */
	min_unit_size = max_t(size_t, size_sum, PCPU_MIN_UNIT_SIZE);

	/* determine the maximum # of units that can fit in an allocation */
	alloc_size = roundup(min_unit_size, atom_size);
	upa = alloc_size / min_unit_size;
	while (alloc_size % upa || (offset_in_page(alloc_size / upa)))
		upa--;
	max_upa = upa;

	/* group cpus according to their proximity */
	for_each_possible_cpu(cpu) {
		group = 0;
	next_group:
		for_each_possible_cpu(tcpu) {
			if (cpu == tcpu)
				break;
		}
		group_map[cpu] = group;
		group_cnt[group]++;
	}

	/*
	 * Wasted space is caused by a ration imbalance of upa to group_cnt.
	 * Expand the unit_size until we use >= 75% of the units allocated.
	 * Related to atom_size, which could be much larger than the unit_size.
	 */
	last_allocs = INT_MAX;
	for (upa = max_upa; upa; upa--) {
		int allocs = 0, wasted = 0;

		if (alloc_size % upa || (offset_in_page(alloc_size / upa)))
			continue;

		for (group = 0; group < nr_groups; group++) {
			int this_allocs = DIV_ROUND_UP(group_cnt[group], upa);
			allocs += this_allocs;
			wasted += this_allocs * upa - group_cnt[group];
		}

		/*
		 * Don't accept if wastage is ove 1/3. The
		 * greater-hean comparison ensures upa==1 always
		 * passes the following check.
		 */
		if (wasted > num_possible_cpus() / 3)
			continue;

		/* and then don't consume more memory */
		if (allocs > last_allocs)
			break;
		last_allocs = allocs;
		best_upa = upa;
	}
	upa = best_upa;

	/* allocated and fill alloc_info */
	for (group = 0; group < nr_groups; group++)
		nr_units += roundup(group_cnt[group], upa);

	ai = pcpu_alloc_alloc_info(nr_groups, nr_units);
	if (!ai)
		return ERR_PTR(-ENOMEM);
	cpu_map = ai->groups[0].cpu_map;

	for (group = 0; group < nr_groups; group++) {
		ai->groups[group].cpu_map = cpu_map;
		cpu_map += roundup(group_cnt[group], upa);
	}

	ai->static_size = static_size;
	ai->reserved_size = reserved_size;
	ai->dyn_size = dyn_size;
	ai->unit_size = alloc_size / upa;
	ai->atom_size = atom_size;
	ai->alloc_size = alloc_size;

	for (group = 0, unit = 0; group_cnt[group]; group++) {
		struct pcpu_group_info *gi = &ai->groups[group];

		/*
		 * Initialize base_offset as if all groups are located
		 * back-to-back. The caller should update this to
		 * reflect actual allocation.
		 */
		gi->base_offset = unit * ai->unit_size;

		for_each_possible_cpu(cpu)
			if (group_map[cpu] == group)
				gi->cpu_map[gi->nr_units++] = cpu;
		gi->nr_units = roundup(gi->nr_units, upa);
		unit += gi->nr_units;
	}

	return ai;
}

/**
 * pcpu_dump_alloc_info - print out information about pcpu_alloc_info
 */
static void pcpu_dump_alloc_info(const struct pcpu_alloc_info *ai)
{
	int group_width = 1, cpu_width = 1, width;
	char empty_str[] = "--------";
	int alloc = 0, alloc_end = 0;
	int group, v;
	int upa, apl;	/* units per alloc, allocs per line */

	v = ai->nr_groups;
	while (v / 10)
		group_width++;

	v = num_possible_cpus();
	if (v /= 10)
		cpu_width++;
	empty_str[min_t(int, cpu_width, sizeof(empty_str) - 1)] = '\0';

	upa = ai->alloc_size / ai->unit_size;
	width = upa * (cpu_width + 1) + group_width + 3;
	apl = rounddown_pow_of_two(max(60 / width, 1));
	
	printk("pcpu-alloc: s%zu r%zu d%zu u%zu alloc=%zu*%zu",
		ai->static_size, ai->reserved_size, ai->dyn_size,
		ai->unit_size, ai->alloc_size / ai->atom_size, ai->atom_size);

	for (group = 0; group < ai->nr_groups; group++) {
		const struct pcpu_group_info *gi = &ai->groups[group];
		int unit = 0, unit_end = 0;

		for (alloc_end += gi->nr_units / upa;
			alloc < alloc_end; alloc++) {
			if (!(alloc % apl)) {
				printk("\n");
				printk("pcpu-alloc: ");
			}
			printk("[%0*d] ", group_width, group);

			for (unit_end += upa; unit < unit_end; unit++)
				if (gi->cpu_map[unit] != NR_CPUS)
					printk("%0*d ",
						cpu_width, gi->cpu_map[unit]);
				else
					printk("%s ", empty_str);
		}
	}
	printk("\n");
}

static inline void pcpu_stats_save_ai(const struct pcpu_alloc_info *ai)
{
	memcpy(&pcpu_stats_ai, ai, sizeof(struct pcpu_alloc_info));

	pcpu_stats.min_alloc_size = pcpu_stats_ai.unit_size;
}

static int __pcpu_size_to_slot(int size)
{
	int highbit = fls(size);	/* size is in bytes */
	return max(highbit - PCPU_SLOT_BASE_SHIFT + 2, 1);
}

static void pcpu_init_md_blocks(struct pcpu_chunk *chunk)
{
	struct pcpu_block_md *md_block;

	for (md_block = chunk->md_blocks;
		md_block != chunk->md_blocks + pcpu_chunk_nr_blocks(chunk);
		md_block++) {
		md_block->contig_hint = PCPU_BITMAP_BLOCK_BITS;
		md_block->left_free = PCPU_BITMAP_BLOCK_BITS;
		md_block->right_free = PCPU_BITMAP_BLOCK_BITS;
	}
}

/**
 * pcpu_cnt_pop_pages - counts populated backing pages in range.
 */
static inline int pcpu_cnt_pop_pages(struct pcpu_chunk *chunk, int bit_off,
					int bits)
{
	int page_start = PFN_UP(bit_off * PCPU_MIN_ALLOC_SIZE);
	int page_end = PFN_DOWN((bit_off + bits) * PCPU_MIN_ALLOC_SIZE);

	if (page_start >= page_end)
		return 0;

	/*
	 * bitmap_weight counts the number of bits set in a bitmap up to
	 * the specified number of bits. This is counting the populated
	 * pages up to page_end and then subtracting the populated pages
	 * up to page_start to count the populated pages in
	 * [page_start, page_end).
	 */
	return bitmap_weight(chunk->populated, page_end) -
		bitmap_weight(chunk->populated, page_start);
}

static unsigned long pcpu_off_to_block_index(int off)
{
	return off / PCPU_BITMAP_BLOCK_BITS;
}

static unsigned long pcpu_off_to_block_off(int off)
{
	return off & (PCPU_BITMAP_BLOCK_BITS - 1);
}

static unsigned long *pcpu_index_alloc_map(struct pcpu_chunk *chunk, int index)
{
	return chunk->alloc_map +
		(index * PCPU_BITMAP_BLOCK_BITS / BITS_PER_LONG);
}

static void pcpu_next_unpop(unsigned long *bitmap, int *rs, int *re, int end)
{
	*rs = find_next_zero_bit(bitmap, end, *rs);
	*re = find_next_bit(bitmap, end, *rs + 1);
}

static unsigned long pcpu_block_off_to_off(int index, int off)
{
	return index * PCPU_BITMAP_BLOCK_BITS + off;
}

static int pcpu_size_to_slot(int size)
{
	if (size == pcpu_unit_size)
		return pcpu_nr_slots - 1;
	return __pcpu_size_to_slot(size);
}

static int pcpu_chunk_slot(const struct pcpu_chunk *chunk)
{
	if (chunk->free_bytes < PCPU_MIN_ALLOC_SIZE || chunk->contig_bits == 0)
		return 0;

	return pcpu_size_to_slot(chunk->free_bytes);
}

#define pcpu_for_each_unpop_region(bitmap, rs, re, start, end)		\
	for ((rs) = (start), pcpu_next_unpop((bitmap), &(rs), &(re), (end)); \
		(rs) < (re);						\
	     (rs) = (re) + 1, pcpu_next_unpop((bitmap), &(rs), &(re), (end)))

static void pcpu_next_md_free_region(struct pcpu_chunk *chunk, int *bit_off,
					int *bits)
{
	int i = pcpu_off_to_block_index(*bit_off);
	int block_off = pcpu_off_to_block_off(*bit_off);
	struct pcpu_block_md *block;

	*bits = 0;
	for (block = chunk->md_blocks + i; i < pcpu_chunk_nr_blocks(chunk);
			block++, i++) {
		/* handles contig area across blocks */
		if (*bits) {
			*bits += block->left_free;
			if (block->left_free == PCPU_BITMAP_BLOCK_BITS)
				continue;
			return;
		}

		/*
		 * This checks three things.  First is there a contig_hint to
		 * check.  Second, have we checked this hint before by
		 * comparing the block_off.  Third, is this the same as the
		 * right contig hint.  In the last case, it spills over into
		 * the next block and should be handled by the contig area
		 * across blocks code.
		 */
		*bits = block->contig_hint;
		if (*bits && block->contig_hint_start >= block_off &&
		  *bits + block->contig_hint_start < PCPU_BITMAP_BLOCK_BITS) {
			*bit_off = pcpu_block_off_to_off(i,
					block->contig_hint_start);
			return;
		}
		/* reset to satisfy the second predicate above */
		block_off = 0;

		*bits = block->right_free;
		*bit_off = (i + 1) * PCPU_BITMAP_BLOCK_BITS -
					block->right_free;
	}
}

/**
 * pcpu_next_fit_region - finds fit areas for a given allocation request
 */
static void pcpu_next_fit_region(struct pcpu_chunk *chunk, int alloc_bits,
			int align, int *bit_off, int *bits)
{
	int i = pcpu_off_to_block_index(*bit_off);
	int block_off = pcpu_off_to_block_off(*bit_off);
	struct pcpu_block_md *block;

	*bits = 0;
	for (block = chunk->md_blocks + i; i < pcpu_chunk_nr_blocks(chunk);
		block++, i++) {
		/* handles contig area accross blocks */
		*bits += block->left_free;
		if (*bits >= alloc_bits)
			return;
		if (block->left_free == PCPU_BITMAP_BLOCK_BITS)
			continue;

		/* check block->contig_hint */
		*bits = ALIGN(block->contig_hint_start, align) - 
			block->contig_hint_start;

		/*
		 * This uses the block offset to determine if this has been
		 * checked in the prior iteration.
		 */
		if (block->contig_hint &&
		    block->contig_hint_start >= block_off &&
		    block->contig_hint >= *bits + alloc_bits) {
			*bits += alloc_bits + block->contig_hint_start - 
					block->first_free;
			*bit_off = pcpu_block_off_to_off(i, block->first_free);
			return;
		}
		/* reset to satisfy the second predicate above */
		block_off = 0;

		*bit_off = ALIGN(PCPU_BITMAP_BLOCK_BITS - block->right_free,
					align);
		*bits = PCPU_BITMAP_BLOCK_BITS - *bit_off;
		*bit_off = pcpu_block_off_to_off(i, *bit_off);
		if (*bits >= alloc_bits)
			return;
	}

	/* no valid offsets were found - fail condition */
	*bit_off = pcpu_chunk_map_bits(chunk);
}

#define pcpu_for_each_md_free_region(chunk, bit_off, bits)		\
	for (pcpu_next_md_free_region((chunk), &(bit_off), &(bits));	\
	    (bit_off) < pcpu_chunk_map_bits((chunk));			\
	    (bit_off) += (bits + 1),					\
	    pcpu_next_md_free_region((chunk), &(bit_off), &(bits)))

#define pcpu_for_each_fit_region(chunk, alloc_bits, align, bit_off, bits) \
	for (pcpu_next_fit_region((chunk), (alloc_bits), (align), &(bit_off), \
					&(bits));			  \
	    (bit_off) < pcpu_chunk_map_bits((chunk));			  \
	    (bit_off) += (bits),					  \
	    pcpu_next_fit_region((chunk), (alloc_bits), (align), &(bit_off), \
					&(bits)))

/**
 * pcpu_block_update - updates a block given a free area
 */
static void pcpu_block_update(struct pcpu_block_md *block, int start, int end)
{
	int contig = end - start;

	block->first_free = min(block->first_free, start);
	if (start == 0)
		block->left_free = contig;

	if (end == PCPU_BITMAP_BLOCK_BITS)
		block->right_free = contig;

	if (contig > block->contig_hint) {
		block->contig_hint_start = start;
		block->contig_hint = contig;
	} else if (block->contig_hint_start && contig == block->contig_hint &&
		  (!start || __ffs(start) > __ffs(block->contig_hint_start))) {
		/* use the start with the best alignment */
		block->contig_hint_start = start;
	}
}

static void pcpu_block_refresh_hint(struct pcpu_chunk *chunk, int index)
{
	struct pcpu_block_md *block = chunk->md_blocks + index;
	unsigned long *alloc_map = pcpu_index_alloc_map(chunk, index);
	int rs, re;	/* region start, region end */

	/* clear hints */
	block->contig_hint = 0;
	block->left_free = block->right_free = 0;

	/* iterate over free areas and update the contig hints */
	pcpu_for_each_unpop_region(alloc_map, rs, re, block->first_free,
					PCPU_BITMAP_BLOCK_BITS) {
		pcpu_block_update(block, rs, re);
	}
}

static void pcpu_chunk_update(struct pcpu_chunk *chunk, int bit_off, int bits)
{
	if (bits > chunk->contig_bits) {
		chunk->contig_bits_start = bit_off;
		chunk->contig_bits = bits;
	} else if (bits == chunk->contig_bits && chunk->contig_bits_start &&
			(!bit_off ||
			__ffs(bit_off) > __ffs(chunk->contig_bits_start))) {
		/* use the start with the best alignment */
		chunk->contig_bits_start = bit_off;
	}
}

static void pcpu_chunk_refresh_hint(struct pcpu_chunk *chunk)
{
	int bit_off, bits, nr_empty_pop_pages;

	/* clear metadata */
	chunk->contig_bits = 0;

	bit_off = chunk->first_bit;
	bits = nr_empty_pop_pages = 0;
	pcpu_for_each_md_free_region(chunk, bit_off, bits) {
		pcpu_chunk_update(chunk, bit_off, bits);

		nr_empty_pop_pages += pcpu_cnt_pop_pages(chunk, bit_off, bits);
	}

	if (chunk != pcpu_reserved_chunk)
		pcpu_nr_empty_pop_pages +=
			(nr_empty_pop_pages - chunk->nr_empty_pop_pages);

	chunk->nr_empty_pop_pages = nr_empty_pop_pages;
}

/**
 * pcpu_block_update_hint_alloc - update hint on allocation path
 */
static void pcpu_block_update_hint_alloc(struct pcpu_chunk *chunk,
				int bit_off, int bits)
{
	struct pcpu_block_md *s_block, *e_block, *block;
	int s_index, e_index; /* block indexes of the freed allocation */
	int s_off, e_off; /* block offsets of the freed allocation */

	/*
	 * Calculate per block offsets.
	 * The calculation uses an inclusive range, but the resulting offsets
	 * are [start, end). e_index always points to the last block in the
	 * range.
	 */
	s_index = pcpu_off_to_block_index(bit_off);
	e_index = pcpu_off_to_block_index(bit_off + bits - 1);
	s_off = pcpu_off_to_block_off(bit_off);
	e_off = pcpu_off_to_block_off(bit_off + bits - 1) + 1;

	s_block = chunk->md_blocks + s_index;
	e_block = chunk->md_blocks + e_index;

	/*
	 * Update s_block.
	 * block->first_free must be updated if the allocation takes its place.
	 * If the allocation breaks the contig_hint, a scan is required to
	 * restore this hint.
	 */
	if (s_off == s_block->first_free)
		s_block->first_free = find_next_zero_bit(
					pcpu_index_alloc_map(chunk, s_index),
					PCPU_BITMAP_BLOCK_BITS,
					s_off + bits);

	if (s_off >= s_block->contig_hint_start &&
	    s_off < s_block->contig_hint_start + s_block->contig_hint) {
		/* block contig hint is broken - scan to fix it */
		pcpu_block_refresh_hint(chunk, s_index);
	} else {
		/* update left and right contig manually */
		s_block->left_free = min(s_block->left_free, s_off);
		if (s_index == e_index)
			s_block->right_free = min_t(int, s_block->right_free,
					PCPU_BITMAP_BLOCK_BITS - e_off);
		else
			s_block->right_free = 0;
	}

	/*
	 * Update e_block
	 */
	if (s_index != e_index) {
		/*
		 * When the allocation is across blocks, the end is along
		 * the left part of the e_block.
		 */
		e_block->first_free = find_next_zero_bit(
				pcpu_index_alloc_map(chunk, e_index),
				PCPU_BITMAP_BLOCK_BITS, e_off);

		if (e_off == PCPU_BITMAP_BLOCK_BITS) {
			/* reset the block */
			e_block++;
		} else {
			if (e_off > e_block->contig_hint_start) {
				/* contig hint is broken - scan to fix it */
				pcpu_block_refresh_hint(chunk, e_index);
			} else {
				e_block->left_free = 0;
				e_block->right_free = 
					min_t(int, e_block->right_free,
					PCPU_BITMAP_BLOCK_BITS - e_off);
			}
		}

		/* update in-between md_blocks */
		for (block = s_block + 1; block < e_block; block++) {
			block->contig_hint = 0;
			block->left_free = 0;
			block->right_free = 0;
		}
	}

	/*
	 * The only time a full chunk scan is required is if the chunk
	 * contig hint is broken. Otherwise, it means a smaller space
	 * was used and therefore the chunk contig hint is still correct.
	 */
	if (bit_off >= chunk->contig_bits_start &&
	    bit_off < chunk->contig_bits_start + chunk->contig_bits)
		pcpu_chunk_refresh_hint(chunk);
}

/**
 * pcpu_alloc_first_chunk - creates chunks that serve the first chunk
 */
static struct pcpu_chunk * pcpu_alloc_first_chunk(unsigned long tmp_addr,
							int map_size)
{
	struct pcpu_chunk *chunk;
	unsigned long aligned_addr, lcm_align;
	int start_offset, offset_bits, region_size, region_bits;

	/* region calculations */
	aligned_addr = tmp_addr & PAGE_MASK;

	start_offset = tmp_addr - aligned_addr;

	/*
	 * Align the end of the region with the LCM of PAGE_SIZE and
	 * PCPU_BITMAP_BLOCK_SIZE. One of these constants is a multiple of
	 * the other.
	 */
	lcm_align = lcm(PAGE_SIZE, PCPU_BITMAP_BLOCK_SIZE);
	region_size = ALIGN(start_offset + map_size, lcm_align);

	/* allocate chunk */
	chunk = memblock_alloc(sizeof(struct pcpu_chunk) +
			BITS_TO_LONGS(region_size >> PAGE_SHIFT),
			SMP_CACHE_BYTES);

	INIT_LIST_HEAD(&chunk->list);

	chunk->base_addr = (void *)aligned_addr;
	chunk->start_offset = start_offset;
	chunk->end_offset = region_size - chunk->start_offset - map_size;

	chunk->nr_pages = region_size >> PAGE_SHIFT;
	region_bits = pcpu_chunk_map_bits(chunk);

	chunk->alloc_map = memblock_alloc(BITS_TO_LONGS(region_bits) * 
				sizeof(chunk->alloc_map[0]), SMP_CACHE_BYTES);
	chunk->bound_map = memblock_alloc(BITS_TO_LONGS(region_bits + 1) *
				sizeof(chunk->bound_map[0]), SMP_CACHE_BYTES);
	chunk->md_blocks = memblock_alloc(pcpu_chunk_nr_blocks(chunk) *
				sizeof(chunk->md_blocks[0]), SMP_CACHE_BYTES);
	pcpu_init_md_blocks(chunk);

	/* manage populated page bitmap */
	chunk->immutable = true;
	bitmap_fill(chunk->populated, chunk->nr_pages);
	chunk->nr_populated = chunk->nr_pages;
	chunk->nr_empty_pop_pages =
		pcpu_cnt_pop_pages(chunk, start_offset / PCPU_MIN_ALLOC_SIZE,
				map_size / PCPU_MIN_ALLOC_SIZE);

	chunk->contig_bits = map_size / PCPU_MIN_ALLOC_SIZE;
	chunk->free_bytes = map_size;

	if (chunk->start_offset) {
		/* hide the beginning of the bitmap */
		offset_bits = chunk->start_offset / PCPU_MIN_ALLOC_SIZE;
		bitmap_set(chunk->alloc_map, 0, offset_bits);
		set_bit(0, chunk->bound_map);
		set_bit(offset_bits, chunk->bound_map);

		chunk->first_bit = offset_bits;

		pcpu_block_update_hint_alloc(chunk, 0, offset_bits);
	}

	if (chunk->end_offset) {
		/* hide the end of the bitmap */
		offset_bits = chunk->end_offset / PCPU_MIN_ALLOC_SIZE;
		bitmap_set(chunk->alloc_map,
			pcpu_chunk_map_bits(chunk) - offset_bits,
			offset_bits);
		set_bit((start_offset + map_size) / PCPU_MIN_ALLOC_SIZE,
			chunk->bound_map);
		set_bit(region_bits, chunk->bound_map);

		pcpu_block_update_hint_alloc(chunk, pcpu_chunk_map_bits(chunk)
					- offset_bits, offset_bits);
	}

	return chunk;
}

/**
 * pcpu_chunk_relocate - put chunk in the appropriate chunk slot
 */
static void pcpu_chunk_relocate(struct pcpu_chunk *chunk, int oslot)
{
	int nslot = pcpu_chunk_slot(chunk);

	if (chunk != pcpu_reserved_chunk && oslot != nslot) {
		if (oslot < nslot)
			list_move(&chunk->list, &pcpu_slot[nslot]);
		else
			list_move_tail(&chunk->list, &pcpu_slot[nslot]);
	}
}

/*
 * pcpu_stats_chunk_alloc - increament chunk stats
 */
static inline void pcpu_stats_chunk_alloc(void)
{
	unsigned long flags;

	pcpu_stats.nr_chunks++;
	pcpu_stats.nr_max_chunks =
		max(pcpu_stats.nr_max_chunks, pcpu_stats.nr_chunks);
}

/**
 * pcpu_setup_first_chunk - initialize the first percpu chunk
 */
int pcpu_setup_first_chunk(const struct pcpu_alloc_info *ai, void *base_addr)
{
	size_t size_sum = ai->static_size + ai->reserved_size + ai->dyn_size;
	size_t static_size, dyn_size;
	struct pcpu_chunk *chunk;
	unsigned long *group_offsets;
	size_t *group_sizes;
	unsigned long *unit_off;
	unsigned int cpu;
	int *unit_map;
	int group, unit, i;
	int map_size;
	unsigned long tmp_addr;

	/* process group information and build config tables accordingly */
	group_offsets = memblock_alloc(ai->nr_groups * sizeof(group_offsets[0]),					SMP_CACHE_BYTES);
	group_sizes = memblock_alloc(ai->nr_groups * sizeof(group_sizes[0]),
					SMP_CACHE_BYTES);
	unit_map = memblock_alloc(nr_cpu_ids * sizeof(unit_map[0]),
					SMP_CACHE_BYTES);
	unit_off = memblock_alloc(nr_cpu_ids * sizeof(unit_off[0]),
					SMP_CACHE_BYTES);

	for (cpu = 0; cpu < nr_cpu_ids; cpu++)
		unit_map[cpu] = UINT_MAX;

	pcpu_low_unit_cpu = NR_CPUS;
	pcpu_high_unit_cpu = NR_CPUS;

	for (group = 0, unit = 0; group < ai->nr_groups; group++, unit += i) {
		const struct pcpu_group_info *gi = &ai->groups[group];

		group_offsets[group] = gi->base_offset;
		group_sizes[group] = gi->nr_units * ai->unit_size;

		for (i = 0; i < gi->nr_units; i++) {
			cpu = gi->cpu_map[i];
			if (cpu == NR_CPUS)
				continue;

			unit_map[cpu] = unit + i;
			unit_off[cpu] = gi->base_offset + i * ai->unit_size;

			/* determine low/high unit_cpu */
			if (pcpu_low_unit_cpu == NR_CPUS ||
			    unit_off[cpu] < unit_off[pcpu_low_unit_cpu])
				pcpu_low_unit_cpu = cpu;
			if (pcpu_high_unit_cpu == NR_CPUS ||
			    unit_off[cpu] > unit_off[pcpu_high_unit_cpu])
				pcpu_high_unit_cpu = cpu;
		}
	}
	pcpu_nr_units = unit;

	pcpu_dump_alloc_info(ai);

	pcpu_nr_groups = ai->nr_groups;
	pcpu_group_offsets = group_offsets;
	pcpu_group_sizes = group_sizes;
	pcpu_unit_map = unit_map;
	pcpu_unit_offsets = unit_off;

	/* determine basic parameters */
	pcpu_unit_pages = ai->unit_size >> PAGE_SHIFT;
	pcpu_unit_size = pcpu_unit_pages << PAGE_SHIFT;
	pcpu_atom_size = ai->atom_size;
	pcpu_chunk_struct_size = sizeof(struct pcpu_chunk) +
		BITS_TO_LONGS(pcpu_unit_pages) * sizeof(unsigned long);

	pcpu_stats_save_ai(ai);

	/*
	 * Allocate chunk slots. The additional last slot is for
	 * empty chunks.
	 */
	pcpu_nr_slots = __pcpu_size_to_slot(pcpu_unit_size) + 2;
	pcpu_slot = memblock_alloc(pcpu_nr_slots * sizeof(pcpu_slot[0]),
					SMP_CACHE_BYTES);
	for (i = 0; i < pcpu_nr_slots; i++)
		INIT_LIST_HEAD(&pcpu_slot[i]);

	/*
	 * The end of the static region needs to be aligned with the
	 * minimum allocation size as this offsets the resrved and
	 * dynamic region. The first chunk ends page aligned by
	 * expanding the dynamic region, therefore the dynamic region
	 * can be shrunk to compensate while still staying above the
	 * configured sizes.
	 */
	static_size = ALIGN(ai->static_size, PCPU_MIN_ALLOC_SIZE);
	dyn_size = ai->dyn_size - (static_size - ai->static_size);

	/*
	 * Initialize first chunk.
	 * If the reserved_size is none-zero, this initializes the reserved
	 * chunk. If the reserved_size is zero, the reserved chunk is NULL
	 * and the dynamic region is initialized here. The first chunk,
	 * pcpu_first_chunk, will always point to the chunk that serves
	 * the dynamic region.
	 */
	tmp_addr = (unsigned long)base_addr + static_size;
	map_size = ai->reserved_size ? : dyn_size;
	chunk = pcpu_alloc_first_chunk(tmp_addr, map_size);

	/* init dynamic chunk if necessary */
	if (ai->reserved_size) {
		pcpu_reserved_chunk = chunk;

		tmp_addr = (unsigned long)base_addr + static_size +
				ai->reserved_size;
		map_size = dyn_size;
		chunk = pcpu_alloc_first_chunk(tmp_addr, map_size);
	}

	/* link the first chunk in */
	pcpu_first_chunk = chunk;
	pcpu_nr_empty_pop_pages = pcpu_first_chunk->nr_empty_pop_pages;
	pcpu_chunk_relocate(pcpu_first_chunk, -1);

	/* include all regions of the first chunk */
	pcpu_nr_populated += PFN_DOWN(size_sum);

	pcpu_stats_chunk_alloc();
	
	/* we're done */
	pcpu_base_addr = base_addr;
	return 0;
}

/* pcpu_free_alloc_info - free percpu allocation info */
void pcpu_free_alloc_info(struct pcpu_alloc_info *ai)
{
	memblock_free_early(__pa(ai), ai->__ai_size);
}

int pcpu_embed_first_chunk(size_t reserved_size, size_t dyn_size,
			   size_t atom_size,
			   pcpu_fc_cpu_distance_fn_t cpu_distance_fn,
			   pcpu_fc_alloc_fn_t alloc_fn,
			   pcpu_fc_free_fn_t free_fn)
{
	void *base = (void *)ULONG_MAX;
	void **areas = NULL;
	struct pcpu_alloc_info *ai;
	size_t size_sum, areas_size;
	unsigned long max_distance;
	int group, i, highest_group, rc;

	ai = pcpu_build_alloc_info(reserved_size, dyn_size, atom_size,
					cpu_distance_fn);
	if (IS_ERR(ai))
		return PTR_ERR(ai);

	size_sum = ai->static_size + ai->reserved_size + ai->dyn_size;
	areas_size = PFN_ALIGN(ai->nr_groups * sizeof(void *));

	areas = memblock_alloc_nopanic(areas_size, L1_CACHE_BYTES);
	if (!areas) {
		rc = -ENOMEM;
		goto out_free;
	}

	/* allocate, copy and determine base address & max_distance */
	highest_group = 0;
	for (group = 0; group < ai->nr_groups; group++) {
		struct pcpu_group_info *gi = &ai->groups[group];
		unsigned int cpu = NR_CPUS;
		void *ptr;

		for (i = 0; i < gi->nr_units && cpu == NR_CPUS; i++)
			cpu = gi->cpu_map[i];

		/* allocate space for the whole group */
		ptr = alloc_fn(cpu, gi->nr_units * ai->unit_size, atom_size);
		if (!ptr) {
			rc = -ENOMEM;
			goto out_free_areas;
		}
		areas[group] = ptr;

		base = min(ptr, base);
		if (ptr > areas[highest_group])
			highest_group = group;
	}
	max_distance = areas[highest_group] - base;
	max_distance += ai->unit_size * ai->groups[highest_group].nr_units;

	/*
	 * Copy data and free unused parts. This should happen after all
	 * allocations are complete; otherwise, we may end up with
	 * overlapping groups.
	 */
	for (group = 0; group < ai->nr_groups; group++) {
		struct pcpu_group_info *gi = &ai->groups[group];
		void *ptr = areas[group];

		for (i = 0; i < gi->nr_units; i++, ptr += ai->unit_size) {
			if (gi->cpu_map[i] == NR_CPUS) {
				/* unused unit, free whole */
				free_fn(ptr, ai->unit_size);
				continue;
			}
			/* copy and return the unused part */
			memcpy(ptr, (void *)__per_cpu_load, ai->static_size);
			free_fn(ptr + size_sum, ai->unit_size - size_sum);
		}
	}

	for (group = 0; group < ai->nr_groups; group++) {
		ai->groups[group].base_offset = areas[group] - base;
	}

	printk("Embedded %zu pages/cpu @%p s%zu d%zu d%zu u%zu\n",
		PFN_DOWN(size_sum), base, ai->static_size, ai->reserved_size,
		ai->dyn_size, ai->unit_size);

	rc = pcpu_setup_first_chunk(ai, base);
	goto out_free;
	
out_free_areas:
	for (group = 0; group < ai->nr_groups; group++)
		if (areas[group])
			free_fn(areas[group],
				ai->groups[group].nr_units * ai->unit_size);
out_free:
	pcpu_free_alloc_info(ai);
	if (areas)
		memblock_free_early(__pa(areas), areas_size);
	return rc;
}

static void *pcpu_dfl_fc_alloc(unsigned int cpu, size_t size, size_t align)
{
	return memblock_alloc_from_nopanic(
				size, align, __pa(MAX_DMA_ADDRESS));
}

static void pcpu_dfl_fc_free(void *ptr, size_t size)
{
	memblock_free_early(__pa(ptr), size);
}

#ifdef CONFIG_SMP
/*
 * Generic SMP percpu area setup.
 *
 * The embedding helper is used because its behavior closely resembles
 * the original non-dynamic generic percpu area setup.  This is
 * important because many archs have addressing restrictions and might
 * fail if the percpu area is located far away from the previous
 * location.  As an added bonus, in non-NUMA cases, embedding is
 * generally a good idea TLB-wise because percpu area can piggy back
 * on the physical linear memory mapping which uses large page
 * mappings on applicable archs.
 */
void setup_per_cpu_areas(void)
{
	unsigned long delta;
	unsigned int cpu;
	int rc;

	/*
	 * Always reserve area for module perpu variables. That's
	 * what the legacy allocator did.
	 */
	rc = pcpu_embed_first_chunk(PERCPU_MODULE_RESERVE,
				    PERCPU_DYNAMIC_RESERVE, PAGE_SIZE, NULL,
				    pcpu_dfl_fc_alloc, pcpu_dfl_fc_free);
	if (rc < 0)
		printk("Failed to initialize perpcu areas.");

	delta = (unsigned long)pcpu_base_addr - (unsigned long)__per_cpu_start;
	for_each_possible_cpu(cpu)
		__per_cpu_offset[cpu] = delta + pcpu_unit_offsets[cpu];
}
#endif

#ifdef CONFIG_UP
/*
 * UP percpu area setup
 *
 * UP always uses km-based percpu allocator with identity mapping.
 * Static percpu variables are indistinguishable from the usual static
 * variables and don't require any special preparation.
 */
void setup_per_cpu_areas(void)
{
	const size_t unit_size =
			roundup_pow_of_two(max_t(size_t, PCPU_MIN_UNIT_SIZE,
					PERCPU_DYNAMIC_RESERVE));
	struct pcpu_alloc_info *ai;
	void *fc;

	ai = pcpu_alloc_alloc_info(1, 1);
	fc = memblock_alloc_from_nopanic(unit_size,
						PAGE_SIZE,
						__pa(MAX_DMA_ADDRESS));
	if (!ai || !fc)
		printk("Failed to allocate memory for percpu areas.\n");

	ai->dyn_size = unit_size;
	ai->unit_size = unit_size;
	ai->atom_size = unit_size;
	ai->alloc_size = unit_size;
	ai->groups[0].nr_units = 1;
	ai->groups[0].cpu_map[0] = 0;

	if (pcpu_setup_first_chunk(ai, fc) < 0)
		printk("Failed to initialize percpu areas.\n");
	pcpu_free_alloc_info(ai);
	printk("UP-PERCPU Allocator Initialized finished.\n");
}
#endif

/**
 * pcpu_is_populated - determines if the region is populated
 */
static bool pcpu_is_populated(struct pcpu_chunk *chunk, int bit_off, int bits,
					int *next_off)
{
	int page_start, page_end, rs, re;

	page_start = PFN_DOWN(bit_off * PCPU_MIN_ALLOC_SIZE);
	page_end = PFN_UP((bit_off + bits) * PCPU_MIN_ALLOC_SIZE);

	rs = page_start;
	pcpu_next_unpop(chunk->populated, &rs, &re, page_end);
	if (rs >= page_end)
		return true;
	*next_off = re * PAGE_SIZE / PCPU_MIN_ALLOC_SIZE;
	return false;
}

/**
 * pcpu_find_block_fit - finds the block index to start searching
 */
static int pcpu_find_block_fit(struct pcpu_chunk *chunk, int alloc_bits,
			size_t align, bool pop_only)
{
	int bit_off, bits, next_off;

	/*
	 * Check to see if the allocation can fit in the chunk's contig hint.
	 * This is an optimization to prevent scanning by assuming if it
	 * cannot fit in the global hint, there is memory pressure and
	 * creating a new chunk would happen soon.
	 */
	bit_off = ALIGN(chunk->contig_bits_start, align) -
			chunk->contig_bits_start;
	if (bit_off + alloc_bits > chunk->contig_bits)
		return  -1;

	bit_off = chunk->first_bit;
	bits = 0;
	pcpu_for_each_fit_region(chunk, alloc_bits, align, bit_off, bits) {
		if (!pop_only || pcpu_is_populated(chunk, bit_off, bits,
							&next_off))
			break;
		bit_off = next_off;
		bits = 0;
	}

	if (bit_off == pcpu_chunk_map_bits(chunk))
		return -1;

	return bit_off;
}

/**
 * pcpu_alloc_area - allocates an area from a pcpu_chunk
 */
static int pcpu_alloc_area(struct pcpu_chunk *chunk, int alloc_bits,
				size_t align, int start)
{
	size_t align_mask = (align) ? (align - 1) : 0;
	int bit_off, end, oslot;

	oslot = pcpu_chunk_slot(chunk);

	/*
	 * Search to find a fit.
	 */
	end = start + alloc_bits + PCPU_BITMAP_BLOCK_BITS;
	bit_off = bitmap_find_next_zero_area(chunk->alloc_map, end, start,
					alloc_bits, align_mask);
	if (bit_off >= end)
		return -1;

	/* update alloc map */
	bitmap_set(chunk->alloc_map, bit_off, alloc_bits);

	/* update boundary map */
	set_bit(bit_off, chunk->bound_map);
	bitmap_clear(chunk->bound_map, bit_off + 1, alloc_bits - 1);
	set_bit(bit_off + alloc_bits, chunk->bound_map);

	chunk->free_bytes -= alloc_bits * PCPU_MIN_ALLOC_SIZE;

	/* update first free bit */
	if (bit_off == chunk->first_bit)
		chunk->first_bit = find_next_zero_bit(chunk->alloc_map,
					pcpu_chunk_map_bits(chunk),
					bit_off + alloc_bits);

	pcpu_block_update_hint_alloc(chunk, bit_off, alloc_bits);

	pcpu_chunk_relocate(chunk, oslot);

	return bit_off * PCPU_MIN_ALLOC_SIZE;
}

/*
 * pcpu_chunk_populate - post-population bookkeeping
 */
static void pcpu_chunk_populated(struct pcpu_chunk *chunk, int page_start,
			int page_end, bool for_alloc)
{
	int nr = page_end - page_start;

	bitmap_set(chunk->populated, page_start, nr);
	chunk->nr_populated += nr;
	pcpu_nr_populated += nr;

	if (!for_alloc) {
		chunk->nr_empty_pop_pages += nr;
		pcpu_nr_empty_pop_pages += nr;
	}
}

static unsigned long pcpu_unit_page_offset(unsigned int cpu, int page_idx)
{
	return pcpu_unit_offsets[cpu] + (page_idx << PAGE_SHIFT);
}

static unsigned long pcpu_chunk_addr(struct pcpu_chunk *chunk,
			unsigned int cpu, int page_idx)
{
	return (unsigned long)chunk->base_addr +
		pcpu_unit_page_offset(cpu, page_idx);
}

/**
 * pcpu_alloc - the percpu allocator
 */
static void __percpu *pcpu_alloc(size_t size, size_t align, bool reserved,
					gfp_t gfp)
{
	/* whitelisted flags that can be passed to the backing allocators */
	gfp_t pcpu_gfp = gfp & (GFP_KERNEL | __GFP_NORETRY | __GFP_NOWARN);
	bool is_atomic = (gfp & GFP_KERNEL) != GFP_KERNEL;
	bool do_warn = !(gfp & __GFP_NOWARN);
	static int warn_limit = 10;
	struct pcpu_chunk *chunk;
	const char *err;
	int slot, off, cpu, ret;
	unsigned long flags;
	void __percpu *ptr;
	size_t bits, bit_align;

	/*
	 * There is now a minimum allocation size of PCPU_MIN_ALLOC_SIZE,
	 * therefore alignment must be a minimum of that many bytes.
	 * An allocation may have internal fragmentation from rounding up
	 * of up to PCPU_MIN_ALLOC_SIZE - 1 bytes.
	 */
	if (unlikely(align < PCPU_MIN_ALLOC_SIZE))
		align = PCPU_MIN_ALLOC_SIZE;

	size = ALIGN(size, PCPU_MIN_ALLOC_SIZE);
	bits = size >> PCPU_MIN_ALLOC_SHIFT;
	bit_align = align >> PCPU_MIN_ALLOC_SHIFT;

	if (unlikely(!size || size > PCPU_MIN_UNIT_SIZE || align > PAGE_SIZE ||
			!is_power_of_2(align))) {
		printk("illegal size (%zu) or align (%zu) for percpu "
			"allocation\n", size, align);
		return NULL;
	}

	/* serve reserved allocations from the reserved chunk if available */
	if (reserved && pcpu_reserved_chunk) {
		chunk = pcpu_reserved_chunk;

		off = pcpu_find_block_fit(chunk, bits, bit_align, is_atomic);
		if (off < 0) {
			err = "alloc from reserved chunk failed";
			goto fail_unlock;
		}

		off = pcpu_alloc_area(chunk, bits, bit_align, off);
		if (off >= 0)
			goto area_found;

		err = "alloc from reserved chunk failed";
		goto fail_unlock;
	}

restart:
	/* search through normal chunks */
	for (slot = pcpu_size_to_slot(size); slot < pcpu_nr_slots; slot++) {
		list_for_each_entry(chunk, &pcpu_slot[slot], list) {
			off = pcpu_find_block_fit(chunk, bits, bit_align,
							is_atomic);
			if (off < 0)
				continue;

			off = pcpu_alloc_area(chunk, bits, bit_align, off);
			if (off >= 0)
				goto area_found;
		}
	}

	/*
	 * No space left. Create a new chunk. We don't want multiple
	 * tasks to create chunks simultaneously. Serialize and create iff
	 * there's still no empty chunk after grabbing the mutex.
	 */
	if (is_atomic) {
		err = "atomic alloc failed, no space left";
		goto fail;
	}

	if (list_empty(&pcpu_slot[pcpu_nr_slots - 1])) {
		printk("NEED NEW.. %s\n", __func__);
	}

	goto restart;

area_found:
	pcpu_stats_area_alloc(chunk, size);

	/* populate if not all pages area already there */
	if (!is_atomic) {
		int page_start, page_end, rs, re;

		page_start = PFN_DOWN(off);
		page_end = PFN_UP(off + size);

		pcpu_for_each_unpop_region(chunk->populated, rs, re,
				page_start, page_end) {
			ret = pcpu_populate_chunk(chunk, rs, re, pcpu_gfp);

			if (ret) {
				printk("NEED ... %s\n", __func__);
				err = "failed to populate";
				goto fail_unlock;
			}
			pcpu_chunk_populated(chunk, rs, re, true);
		}	
	}

	if (pcpu_nr_empty_pop_pages < PCPU_EMPTY_POP_PAGES_LOW)
		printk("NEED schedule...%s\n", __func__);

	/* clear the areas and return address relative to base address */
	for_each_possible_cpu(cpu)
		memset((void *)pcpu_chunk_addr(chunk, cpu, 0) + off, 0, size);

	ptr = __addr_to_pcpu_ptr(chunk->base_addr + off);

	return ptr;

fail_unlock:
	/* no lock */;
fail:
	return NULL;
}

/**
 * pcpu_addr_in_chunk - check if the address is served from this chunk
 */
static bool pcpu_addr_in_chunk(struct pcpu_chunk *chunk, void *addr)
{
	void *start_addr, *end_addr;

	if (!chunk)
		return false;

	start_addr = chunk->base_addr + chunk->start_offset;
	end_addr = chunk->base_addr + chunk->nr_pages * PAGE_SIZE -
			chunk->end_offset;
	return addr >= start_addr && addr < end_addr;
}

/**
 * pcpu_chunk_addr_search - determine chunk containing specified address.
 */
static struct pcpu_chunk *pcpu_chunk_addr_search(void *addr)
{
	/* is it in the dynamic region (first chunk)? */
	if (pcpu_addr_in_chunk(pcpu_first_chunk, addr))
		return pcpu_first_chunk;

	/* is it in the reserved region */
	if (pcpu_addr_in_chunk(pcpu_reserved_chunk, addr))
		return pcpu_reserved_chunk;

	/*
	 * The address is relative to unit0 which might be unused and
	 * thus unmapped. Offset the address to the unit space of the
	 * current processor before looking it up in the vmalloc
	 * space. Note that any possible cpu id can be used here. so
	 * there's no need to worry about preempting or cpu hotplug.
	 */
	addr += pcpu_unit_offsets[0];
	printk("NEED... %s\n", __func__);
	return NULL;
}

/**
 * pcpu_block_update_hint_free - updates the block hints on the free path
 * @chunk: chunk of interest
 * @bit_off: chunk offset
 * @bits: size of request
 *
 * Updates metadata for the allocation path.  This avoids a blind block
 * refresh by making use of the block contig hints.  If this fails, it scans
 * forward and backward to determine the extent of the free area.  This is
 * capped at the boundary of blocks.
 *
 * A chunk update is triggered if a page becomes free, a block becomes free,
 * or the free spans across blocks.  This tradeoff is to minimize iterating
 * over the block metadata to update chunk->contig_bits.  chunk->contig_bits
 * may be off by up to a page, but it will never be more than the available
 * space.  If the contig hint is contained in one block, it will be accurate.
 */
static void pcpu_block_update_hint_free(struct pcpu_chunk *chunk,
					int bit_off, int bits)
{
	struct pcpu_block_md *s_block, *e_block, *block;
	int s_index, e_index;	/* block indexes of the freed allocation */
	int s_off, e_off;	/* block offsets of the freed allocation */
	int start, end;		/* start and end of the whole free area */

	/*
	 * Calculate per block offsets.
	 * The calculation uses an inclusive range, but the resulting offsets
	 * are [start, end). e_index always points to the last block in the
	 * range.
	 */
	s_index = pcpu_off_to_block_index(bit_off);
	e_index = pcpu_off_to_block_index(bit_off + bits - 1);
	s_off = pcpu_off_to_block_off(bit_off);
	e_off = pcpu_off_to_block_off(bit_off + bits - 1) + 1;

	s_block = chunk->md_blocks + s_index;
	e_block = chunk->md_blocks + e_index;

	/*
	 * Check if the freed area aligns with the block->contig_hint.
	 * If it does, then the scan to find the beginning/end of the 
	 * larger free area can be avoided.
	 *
	 * start and end refer to beginning and end of the free area
	 * within each their respective blocks. This is not necessarily
	 * the entire free area as it may span blocks past the begining
	 * or end of the block.
	 */
	start = s_off;
	if (s_off == s_block->contig_hint + s_block->contig_hint_start) {
		start = s_block->contig_hint_start;
	} else {
		/*
		 * Scan backward to find the extent of the free area.
		 * find_last_bit returns the starting bit, so if the start bit
		 * is returned, that means there was no last bit and the
		 * remainder of the chunk is free.
		 */
		int l_bit = find_last_bit(pcpu_index_alloc_map(chunk, s_index),
						start);
		start = (start == l_bit) ? 0 : l_bit + 1;
	}

	end = e_off;
	if (e_off == e_block->contig_hint_start)
		end = e_block->contig_hint_start + e_block->contig_hint;
	else
		end = find_next_bit(pcpu_index_alloc_map(chunk, e_index),
				PCPU_BITMAP_BLOCK_BITS, end);

	/* update s_block */
	e_off = (s_index == e_index) ? end : PCPU_BITMAP_BLOCK_BITS;
	pcpu_block_update(s_block, start, e_off);

	/* freeing in the same block */
	if (s_index != e_index) {
		/* update e_block */
		pcpu_block_update(e_block, 0, end);

		/* reset md_blocks in the middle */
		for (block = s_block + 1; block < e_block; block++) {
			block->first_free = 0;
			block->contig_hint_start = 0;
			block->contig_hint = PCPU_BITMAP_BLOCK_BITS;
			block->left_free = PCPU_BITMAP_BLOCK_BITS;
			block->right_free = PCPU_BITMAP_BLOCK_BITS;
		}
	}

	/*
	 * Refresh chunk metadata when the free makes a page free, a block
	 * free, or spans across blocks. The contig hint may be off by up to
	 * a page, but if the hint is contained in a block, it will be accurate
	 * with the else condition below.
	 */
	if ((ALIGN_DOWN(end, min(PCPU_BITS_PER_PAGE, 
				PCPU_BITMAP_BLOCK_BITS)) > ALIGN(start,
				min(PCPU_BITS_PER_PAGE, 
				PCPU_BITMAP_BLOCK_BITS))) ||
				s_index != e_index)
		pcpu_chunk_refresh_hint(chunk);
	else
		pcpu_chunk_update(chunk, pcpu_block_off_to_off(s_index,
					start), s_block->contig_hint);
}

/**
 * pcpu_free_area - frees the corresponding offset
 */
static void pcpu_free_area(struct pcpu_chunk *chunk, int off)
{
	int bit_off, bits, end, oslot;

	pcpu_stats_area_dealloc(chunk);

	oslot = pcpu_chunk_slot(chunk);

	bit_off = off / PCPU_MIN_ALLOC_SIZE;

	/* find end index */
	end = find_next_bit(chunk->bound_map, pcpu_chunk_map_bits(chunk),
				bit_off + 1);
	bits = end - bit_off;
	bitmap_clear(chunk->alloc_map, bit_off, bits);

	/* update metadata */
	chunk->free_bytes += bits * PCPU_MIN_ALLOC_SIZE;

	/* update first free bit */
	chunk->first_bit = min(chunk->first_bit, bit_off);

	pcpu_block_update_hint_free(chunk, bit_off, bits);

	pcpu_chunk_relocate(chunk, oslot);
}

/**
 * free_percpu - free percpu area
 */
void free_percpu(void __percpu *ptr)
{
	void *addr;
	struct pcpu_chunk *chunk;
	unsigned long flags;
	int off;

	if (!ptr)
		return;

	addr = __pcpu_ptr_to_addr(ptr);	

	chunk = pcpu_chunk_addr_search(addr);
	off = addr - chunk->base_addr;

	pcpu_free_area(chunk, off);

	/* if there are more than one fully free chunks, wake up grim reaper */
	if (chunk->free_bytes == pcpu_unit_size) {
		struct pcpu_chunk *pos;

		list_for_each_entry(pos, &pcpu_slot[pcpu_nr_slots - 1], list)
			if (pos != chunk) {
				printk("Need %s\n", __func__);
				break;
			}
	}
}

/**
 * __alloc_percpu - allocate dynamic percpu area
 */
void __percpu *__alloc_percpu(size_t size, size_t align)
{
	return pcpu_alloc(size, align, false, GFP_KERNEL);
}
