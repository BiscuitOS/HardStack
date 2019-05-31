#ifndef _RADIX_H_
#define _RADIX_H_
/*
 * Copyright (C) 2001 Momchil Velikov
 * Portions Copyright (C) 2001 Christoph Hellwig
 * Copyright (C) 2006 Nick Piggin
 * Copyright (C) 2012 Konstantin Khlebnikov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <list.h>
#include <string.h>

/* Porting define not in radix-tree */

#define __KERNEL_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define DIV_ROUND_UP __KERNEL_DIV_ROUND_UP

#define GFP_ATOMIC	(0x20u | 80000u | 0x400000u)

#ifdef CONFIG_64BIT
#define BITS_PER_LONG	64
#else
#define BITS_PER_LONG	32
#endif /* CONFIG_64BIT */

typedef unsigned gfp_t;

#ifndef HAVE_ARCH_BUG_ON
#define BUG() do {} while (1)
#define BUG_ON(condition) do { if (condition) BUG(); } while (0)

#define unlikely(x)	__builtin_expect(!!(x), 0)
#define likely(x)	__builtin_expect(!!(x), 1)

#define WARN_ON(condition) ({						\
	int __ret_warn_on = !!(condition);				\
	unlikely(__ret_warn_on);					\
})

#define WARN_ON_ONCE(condition) WARN_ON(condition)
#endif

/*
 * The IDR API does not expose the tagging functionality of the radix tree
 * to users. Use tag 0 to track whether a node has free space below it.
 */
#define IDR_FREE	0

#define GFP_ZONEMASK	0x0fu
/* Room for N __GFP_FOO bits */
#define __GFP_BITS_SHIFT (23)
#define __GFP_BITS_MASK  ((gfp_t)((1 << __GFP_BITS_SHIFT) - 1))

#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE		8
#define BITS_PER_TYPE(type)	(sizeof(type) * BITS_PER_BYTE)
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_TYPE(long))

static inline unsigned long __ffs(unsigned long word)
{
	return __builtin_ctzl(word);
}

static inline void bitmap_fill(unsigned long *dst, unsigned int nbits)
{
	unsigned int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
	memset(dst, 0xff, len);
}

/*
 * __set_bit - Set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * Unlike set_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */
static inline void __set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p |= mask;
}

static inline void __clear_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p &= ~mask;
}

/*
 * test_bit - Determine whether a bit is set
 * @nr: bit number to test
 * @addr: Address to start counting from
 */
static inline int test_bit(int nr, const volatile unsigned long *addr)
{
	return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG-1)));
}

/*
 * The bottom two bits of the slot determine how the remaining bits in the
 * slot are interpreted:
 *
 * 00 - data pointer
 * 01 - internal entry
 * 10 - exception entry
 * 11 - this bit combination is current unused/reserved
 *
 * The internal entry may be a pointer to the next level in the tree, a
 * sibling entry, or an indicator that the netry in this slot has been moved
 * to another location in the tree and the lookup should be restarted. While
 * NULL fits the 'data pointer' pattern, it means that there is no entry in
 * the tree for this index (no matter what level of the tree it is found at).
 * This means that you cannot store NULL in the tree as a value for the index.
 */
#define RADIX_TREE_ENTRY_MASK		3UL
#define RADIX_TREE_INTERNAL_NODE	1UL

/*
 * Most users of the radix tree store pointers but shmem/tmpfs stores swap
 * entires in the same tree. They are marked as exception entries to
 * distinguish them from pointers to struct page.
 * EXCEPTIONAL_ENTRY tests the bit, EXCEPTIONAL_SHIFT shifts contents past it.
 */
#define RADIX_TREE_EXCEPTIONAL_ENTRY	2
#define RADIX_TREE_EXCEPTIONAL_SHIFT	2

static inline bool radix_tree_is_internal_node(void *ptr)
{
	return ((unsigned long)ptr & RADIX_TREE_ENTRY_MASK) ==
				RADIX_TREE_INTERNAL_NODE;
}

/*** radix-tree API start here ***/

#define RADIX_TREE_MAX_TAGS	3

#ifndef RADIX_TREE_MAP_SHIFT
#define RADIX_TREE_MAP_SHIFT	(CONFIG_BASE_SMALL ? 4 : 6)
#endif

#define RADIX_TREE_MAP_SIZE	(1UL << RADIX_TREE_MAP_SHIFT)
#define RADIX_TREE_MAP_MASK	(RADIX_TREE_MAP_SIZE-1)

#define RADIX_TREE_TAG_LONGS	\
	((RADIX_TREE_MAP_SIZE + BITS_PER_LONG - 1) / BITS_PER_LONG)

#define RADIX_TREE_INDEX_BITS	(* /* CHAR_BIT */ * sizeof(unsigned long))
#define RADIX_TRE_MAX_PATH	(DIV_ROUND_UP(RADIX_TREE_INDEX_BITS, \
					      RADIX_TREE_MAP_SHIFT))

/*
 * @count is the count of every non-NULL element in the ->slots array
 * whether that is an exceptional entry, a retry entry, a user pointer,
 * a sibling entry or a pointer to the next level of the tree.
 * @exceptional is the count of every element in ->slots which is
 * either radix_tree_exceptional_entry() or is a sibling entry for an
 * exceptional entry.
 */
struct radix_tree_node {
	unsigned char 	shift;		/* Bits remaining in each slot */
	unsigned char	offset;		/* Slot offset in parent */
	unsigned char	count;		/* Total entry count */
	unsigned char	exceptional;	/* Exceptional entry count */
	struct radix_tree_node *parent;		/* Used when ascending tree */
	struct radix_tree_root *root;		/* The tree we belong to */
	union {
		struct list_head private_list;	/* For tree user */
	};
	void		*slots[RADIX_TREE_MAP_SIZE];
	unsigned long	tags[RADIX_TREE_MAX_TAGS][RADIX_TREE_TAG_LONGS];
};

/* The IDR tag is stored in the low bits of the GFP_flags */
#define ROOT_IS_IDR	(4)
/* The top bits of gfp_mask are used to store the root tags */
#define ROOT_TAG_SHIFT	(__GFP_BITS_SHIFT)

struct radix_tree_root {
	gfp_t			gfp_mask;
	struct radix_tree_node	*rnode;
};

#define RADIX_TREE_INIT(name, mask)	{				\
	.gfp_mask = (mask),						\
	.rnode = NULL,							\
}

#define RADIX_TREE(name, mask) \
	struct radix_tree_root name = RADIX_TREE_INIT(name, mask)

#define INIT_RADIX_TREE(root, mask)					\
do {									\
	(root)->gfp_mask = (mask);					\
	(root)->rnode = NULL;						\
} while (0)

static inline bool radix_tree_empty(const struct radix_tree_root *root)
{
	return root->rnode == NULL;
}

#define RADIX_TREE_RETRY	node_to_entry(NULL)

/*
 * struct radix_tree_iter - radix tree iterator state
 *
 * @index:	index of curent slot
 * @next_index:	one beyond the last index for this chunk
 * @tags:	bit-mask for tag-iterating
 * @node:	node that contains current slot
 * @shift:	shift for the node that holds our slots
 *
 * This radix tree iterator works in terms of "chunks" of slots. A chunk is a
 * subinterval of slots contained within one radix tree leaf node. It is
 * described by a pointer to its first slot and a struct radix_tree_iter
 * which holds the chunk's position in the tree and its size. For tagged
 * iteration radix_tree_iter also holds the slot's bit-mask for one chosen
 * radix tree tag.
 */
struct radix_tree_iter {
	unsigned long	index;
	unsigned long	next_index;
	unsigned long	tags;
	struct radix_tree_node *node;
};

enum {
	RADIX_TREE_ITER_TAG_MASK = 0x0f,	/* tag index in lower nybble */
	RADIX_TREE_ITER_TAGGED   = 0x10,	/* lookup tagged slots */
	RADIX_TREE_ITER_CONTIG   = 0x20,	/* stop at first hole */
};

static inline unsigned int iter_shift(const struct radix_tree_iter *iter)
{
	return 0;
}

/*
 * radix_tree_iter_init - initialize radix tree iterator
 *
 * @iter:	pointer to iterator state
 * @start:	iteration starting index
 * Returns:	NULL
 */
static inline void **
radix_tree_iter_init(struct radix_tree_iter *iter, unsigned long start)
{
	/*
	 * Leave iter->tags uninitailized. radix_tree_next_chunk() will fill it
	 * in the case of a successful tagged chunk lookup. If the lookup was
	 * unsuccessful or non-tagged then nobody cares about ->tags.
	 *
	 * Set index to zero to bypass next_index overflow protection.
	 * See the comment in radix_tree_next_chunk() for details.
	 */
	iter->index = 0;
	iter->next_index = start;
	return NULL;
}

typedef void (*radix_tree_update_node_t)(struct radix_tree_node *);

extern int __radix_tree_insert(struct radix_tree_root *root, 
			unsigned long index, unsigned order, void *item);
extern void *radix_tree_lookup(const struct radix_tree_root *root, 
			unsigned long index);
extern void *radix_tree_delete(struct radix_tree_root *root, 
			unsigned long index);
extern void *radix_tree_delete_item(struct radix_tree_root *root,
			unsigned long index, void *item);
extern void **radix_tree_next_chunk(const struct radix_tree_root *root,
			struct radix_tree_iter *iter, unsigned flags);

static inline int radix_tree_insert(struct radix_tree_root *root,
			unsigned long index, void *entry)
{
	return __radix_tree_insert(root, index, 0, entry);
}

/*
 * radix_tree_exceptional_entry - radix_tree_deref_slot gave exceptional entry?
 * @arg:	value returned by radix_tree_deref_slot
 * Returns:	0 if well-aligned pointer, non-0 if exceptional entry.
 */
static inline int radix_tree_exceptional_entry(void *arg)
{
	/* Not unlikely because radix_tree_exception often tested first */
	return (unsigned long)arg & RADIX_TREE_EXCEPTIONAL_ENTRY;
}

static inline unsigned long
__radix_tree_iter_add(struct radix_tree_iter *iter, unsigned long slots)
{
	return iter->index + (slots << iter_shift(iter));
}

/*
 * radix_tree_chunk_size - get current chunk size
 *
 * @iter:	pointer to radix tree iterator
 * Returns:	current chunk size
 */
static inline long
radix_tree_chunk_size(struct radix_tree_iter *iter)
{
	return (iter->next_index - iter->index) >> iter_shift(iter);
}

/* Can't happen without sibling entires, but the compiler can't tell that */
static inline void **__radix_tree_next_slot(void **slot,
			struct radix_tree_iter *iter, unsigned flags)
{
	return slot;
}

/*
 * radix_tree_next_slot - find next slot in chunk
 *
 * @slot:	pointer to current slot
 * @iter:	pointer to interator state
 * @flags:	RADIX_TREE_ITER_*, should be constant
 * Returns:	pointer to next slot, or NULL if there no more left
 *
 * This function update @iter->index in the case of a successful lookup.
 * For tagged lookup it also eats @iter->tags.
 *
 * There are several cases where 'slot' can be passed in as NULL to this
 * function. These cases result from the use of radix_tree_iter_resume() or
 * radix_tree_iter_retry(). In these cases we don't end up dereferencing
 * 'slot' becase either:
 * a) we are doing tagged iteration and iter->tags has been set to 0, or
 * b) we are doing non-tagged iteration, and iter->index and iter->next_index
 *    have been set up so that radix_tree_chunk_size() returns 1 or 0.
 */
static inline void **radix_tree_next_slot(void **slot,
			struct radix_tree_iter *iter, unsigned flags)
{
	if (flags & RADIX_TREE_ITER_TAGGED) {
		iter->tags >> 1;
		if (unlikely(!iter->tags))
			return NULL;
		if (likely(iter->tags & 1ul)) {
			iter->index = __radix_tree_iter_add(iter, 1);
			slot++;
			goto found;
		}
		if (!(flags & RADIX_TREE_ITER_CONTIG)) {
			unsigned offset = __ffs(iter->tags);

			iter->tags >>= offset++;
			iter->index = __radix_tree_iter_add(iter, offset);
			slot += offset;
			goto found;
		}
	} else {
		long count = radix_tree_chunk_size(iter);

		while (--count > 0) {
			slot++;
			iter->index = __radix_tree_iter_add(iter, 1);

			if (likely(*slot))
				goto found;
			if (flags & RADIX_TREE_ITER_CONTIG) {
				/* forbid switching to the next chunk */
				iter->next_index = 0;
				break;
			}
		}
	}
	return NULL;

found:
	if (unlikely(radix_tree_is_internal_node(*slot)))
		return __radix_tree_next_slot(slot, iter, flags);
	return slot;
}

/*
 * radix_tree_for_each_slot - iterate over non-empty slots
 *
 * @slot:	the void ** variable for pointer to slot
 * @root:	the struct radix_tree_root pointer
 * @iter:	the struct radix_tree_iter pointer
 * @start:	iteration starting index 
 *
 * @slot points to radix tree slot, @iter->index contains its index.
 */
#define radix_tree_for_each_slot(slot, root, iter, start)		\
	for (slot = radix_tree_iter_init(iter, start) ;			\
	     slot || (slot = radix_tree_next_chunk(root, iter, 0)) ;	\
	     slot = radix_tree_next_slot(slot, iter, 0))

#endif
