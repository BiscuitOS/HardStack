/*
 * Copyright (C) 2001 Momchil Velikov
 * Portions Copyright (C) 2001 Christoph Hellwig
 * Copyright (C) 2005 SGI, Christoph Lameter
 * Copyright (C) 2006 Nick Piggin
 * Copyright (C) 2012 Konstantin Khlebnikov
 * Copyright (C) 2016 Intel, Matthew Wilcox
 * Copyright (C) 2016 Intel, Ross Zwisler
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
#include <errno.h>
#include <malloc.h>

#include <radix.h>

/*
 * Per-cpu pool of perloaded nodes
 */
struct radix_tree_preload {
	unsigned nr;
	/* nodes->parent points to next preallocated node */
	struct radix_tree_node *nodes;
};

/*
 * The IDR does not have to be as high as the radix tree since it uses
 * signed integers, not unsigned longs.
 */
#define IDR_INDEX_BITS		(8 /* CHAR_BIT */ * sizeof(int) - 1)
#define IDR_MAX_PATH		(DIV_ROUND_UP(IDR_INDEX_BITS, \
						RADIX_TREE_MAP_SHIFT))

#define IDR_PRELOAD_SIZE	(IDR_MAX_PATH * 2 - 1)

static struct radix_tree_preload radix_tree_preloads = { 0, };

static inline void *node_to_entry(void *ptr)
{
	return (void *)((unsigned long)ptr | RADIX_TREE_INTERNAL_NODE);
}

static inline gfp_t root_gfp_mask(const struct radix_tree_root *root)
{
	return root->gfp_mask & (__GFP_BITS_MASK & ~GFP_ZONEMASK);
}

static inline struct radix_tree_node *entry_to_node(void *ptr)
{
	return (void *)((unsigned long)ptr & ~RADIX_TREE_INTERNAL_NODE);
}

static inline unsigned long shift_maxindex(unsigned int shift)
{
	return (RADIX_TREE_MAP_SIZE << shift) - 1;
}

static inline unsigned long node_maxindex(const struct radix_tree_node *node)
{
	return shift_maxindex(node->shift);
}

static unsigned radix_tree_load_root(const struct radix_tree_root *root,
		struct radix_tree_node **nodep, unsigned long *maxindex)
{
	struct radix_tree_node *node = root->rnode;

	*nodep = node;

	if (radix_tree_is_internal_node(node)) {
		node = entry_to_node(node);
		*maxindex = node_maxindex(node);
		return node->shift + RADIX_TREE_MAP_SHIFT;
	}

	*maxindex = 0;
	return 0;
}

static inline bool is_idr(const struct radix_tree_root *root)
{
	return !!(root->gfp_mask & ROOT_IS_IDR);
}

static inline void root_tag_set(struct radix_tree_root *root, unsigned tag)
{
	root->gfp_mask |= (gfp_t)(1 << (tag + ROOT_TAG_SHIFT));
}

static inline int root_tag_get(const struct radix_tree_root *root, unsigned tag)
{
	return (int)root->gfp_mask & (1 << (tag + ROOT_TAG_SHIFT));
}

static inline unsigned root_tags_get(const struct radix_tree_root *root)
{
	return (unsigned)root->gfp_mask >> ROOT_TAG_SHIFT;
}

static inline void all_tag_set(struct radix_tree_node *node, unsigned int tag)
{
	bitmap_fill(node->tags[tag], RADIX_TREE_MAP_SIZE);
}

static inline void tag_set(struct radix_tree_node *node, unsigned int tag,
			int offset)
{
	__set_bit(offset, node->tags[tag]);
}

static inline int tag_get(const struct radix_tree_node *node, unsigned int tag,
			int offset)
{
	return test_bit(offset, node->tags[tag]);
}

static inline void tag_clear(struct radix_tree_node *node, unsigned int tag,
			int offset)
{
	__clear_bit(offset, node->tags[tag]);
}

static inline unsigned long
get_slot_offset(const struct radix_tree_node *parent, void **slot)
{
	return parent ? slot - parent->slots : 0;
}

static inline void root_tag_clear(struct radix_tree_root *root, unsigned tag)
{
	root->gfp_mask &= (gfp_t)~(1 << (tag + ROOT_TAG_SHIFT));
}

static inline void root_tag_clear_all(struct radix_tree_root *root)
{
	root->gfp_mask &= (1 << ROOT_TAG_SHIFT) - 1;
}

static bool node_tag_get(const struct radix_tree_root *root,
				const struct radix_tree_node *node,
				unsigned int tag, unsigned int offset)
{
	if (node)
		return tag_get(node, tag, offset);
	return root_tag_get(root, tag);
}

/*
 * Returns 1 if any slot in the node has this tag set.
 * Otherwise returns 0.
 */
static inline int any_tag_set(const struct radix_tree_node *node,
							unsigned int tag)
{
	unsigned idx;
	for (idx = 0; idx < RADIX_TREE_TAG_LONGS; idx++) {
		if (node->tags[tag][idx])
			return 1;
	}
	return 0;
}

static void node_tag_set(struct radix_tree_root *root,
				struct radix_tree_node *node,
				unsigned int tag, unsigned int offset)
{
	while (node) {
		if (tag_get(node, tag, offset))
			return;
		tag_set(node, tag, offset);
		offset = node->offset;
		node = node->parent;
	}

	if (!root_tag_get(root, tag))
		root_tag_set(root, tag);
}

static void node_tag_clear(struct radix_tree_root *root,
				struct radix_tree_node *node,
				unsigned int tag, unsigned int offset)
{
	while (node) {
		if (!tag_get(node, tag, offset))
			return;
		tag_clear(node, tag, offset);
		if (any_tag_set(node, tag))
			return;

		offset = node->offset;
		node = node->parent;
	}

	/* clear the root's tag bit */
	if (root_tag_get(root, tag))
		root_tag_clear(root, tag);
}

/*
 * This assumes that the caller has performed appropriate preallocation, and
 * that the caller has pinned this thread of control to the current CPU.
 */
static struct radix_tree_node *
radix_tree_node_alloc(gfp_t gfp_mask, struct radix_tree_node *parent,
			struct radix_tree_root *root,
			unsigned int shift, unsigned int offset,
			unsigned int count, unsigned int exceptional)
{
	struct radix_tree_node *ret = NULL;

	/* On Application, directly allocate memory from malloc() */
	ret = (struct radix_tree_node *)malloc(sizeof(struct radix_tree_node));
	BUG_ON(radix_tree_is_internal_node(ret));
	if (ret) {
		ret->shift = shift;
		ret->offset = offset;
		ret->count = count;
		ret->exceptional = exceptional;
		ret->parent = parent;
		ret->root = root;
	}
	return ret;
}

/*
 * Extend a radix tree so it can store key @index.
 */
static int radix_tree_extend(struct radix_tree_root *root, gfp_t gfp,
				unsigned long index, unsigned int shift)
{
	void *entry;
	unsigned int maxshift;
	int tag;

	/* Figure out what the shift should be. */
	maxshift = shift;
	while (index > shift_maxindex(maxshift)) {
		maxshift += RADIX_TREE_MAP_SHIFT;
	}

	entry = root->rnode;
	if (!entry && (!is_idr(root) || root_tag_get(root, IDR_FREE)))
		goto out;

	do {
		struct radix_tree_node *node = radix_tree_node_alloc(gfp, NULL,
							root, shift, 0, 1, 0);
		if (!node)
			return -ENOMEM;

		if (is_idr(root)) {
			all_tag_set(node, IDR_FREE);
			if (!root_tag_get(root, IDR_FREE)) {
				tag_clear(node, IDR_FREE, 0);
				root_tag_set(root, IDR_FREE);
			}
		} else {
			/* Propagate the aggregated tag info to the new child */
			for (tag = 0; tag < RADIX_TREE_MAX_TAGS; tag++) {
				if (root_tag_get(root, tag))
					tag_set(node, tag, 0);
			}
		}

		BUG_ON(shift > BITS_PER_LONG);
		if (radix_tree_is_internal_node(entry)) {
			entry_to_node(entry)->parent = node;
		} else if (radix_tree_exceptional_entry(entry)) {
			/* Moving an exceptional root->rnode to a node */
			node->exceptional = 1;
		}
		/*
		 * entry was already in the radix tree, so we do not need
		 * rcu_assign_pointer here.
		 */
		node->slots[0] = entry;
		entry = node_to_entry(node);
		root->rnode = entry;
		shift += RADIX_TREE_MAP_SHIFT;
	} while (shift <= maxshift);
out:
	return maxshift + RADIX_TREE_MAP_SHIFT;
}

static unsigned int radix_tree_descend(const struct radix_tree_node *parent,
			struct radix_tree_node **nodep, unsigned long index)
{
	unsigned int offset = (index >> parent->shift) & RADIX_TREE_MAP_MASK;
	void **entry = parent->slots[offset];

	*nodep = (void *)entry;
	return offset;
}

/*
 * __radix_tree_create -	create a slot in a radix tree
 * @root:	radix tree root
 * @index:	index key
 * @order:	index occupies 2^order aligned slots
 * @nodep:	return node
 * @slotp:	return slot
 *
 * Create, if necessary, and return the node and slot fro an item
 * at position @index in the radix tree @root.
 *
 * Until there is more than one item in the three, no nodes are
 * allocated an @root->rnode is used as a direct slot instead of
 * pointing to a node, in which case *@nodep will be NULL.
 *
 * Returns -ENOMEM, or 0 for success.
 */
int __radix_tree_create(struct radix_tree_root *root, unsigned long index,
			unsigned order, struct radix_tree_node **nodep,
			void ***slotp)
{
	struct radix_tree_node *node = NULL, *child;
	void **slot = (void **)&root->rnode;
	unsigned long maxindex;
	unsigned int shift, offset = 0;
	unsigned long max = index | ((1UL << order) - 1);
	gfp_t gfp = root_gfp_mask(root);

	shift = radix_tree_load_root(root, &child, &maxindex);

	/* Make sure the tree is high enough. */
	if (order > 0 && max == ((1UL << order) - 1))
		max++;
	if (max > maxindex) {
		int error = radix_tree_extend(root, gfp, max, shift);
		if (error < 0)
			return error;
		shift = error;
		child = root->rnode;
	}

	while (shift > order) {
		shift -= RADIX_TREE_MAP_SHIFT;
		if (child == NULL) {
			/* Have to add a child node. */
			child = radix_tree_node_alloc(gfp, node, root, shift,
							offset, 0, 0);
			if (!child)
				return -ENOMEM;
			*slot = node_to_entry(child);
			if (node)
				node->count++;
		} else if (!radix_tree_is_internal_node(child))
			break;

		/* Go a level down */
		node = entry_to_node(child);
		offset = radix_tree_descend(node, &child, index);
		slot = &node->slots[offset];
	}

	if (nodep)
		*nodep = node;
	if (slotp)
		*slotp = slot;
	return 0;
}

static inline int insert_entires(struct radix_tree_node *node,
		void **slot, void *item, unsigned order, bool replace)
{
	if (*slot)
		return -EEXIST;
	*slot = item;
	if (node) {
		node->count++;
		if (radix_tree_exceptional_entry(item))
			node->exceptional++;
	}
	return 1;
}

int __radix_tree_insert(struct radix_tree_root *root, unsigned long index,
			unsigned order, void *item)
{
	struct radix_tree_node *node;
	void **slot;
	int error;

	BUG_ON(radix_tree_is_internal_node(item));

	error = __radix_tree_create(root, index, order, &node, &slot);
	if (error)
		return error;

	error = insert_entires(node, slot, item, order, false);
	if (error < 0)
		return error;

	if (node) {
		unsigned offset = get_slot_offset(node, slot);
		BUG_ON(tag_get(node, 0, offset));
		BUG_ON(tag_get(node, 1, offset));
		BUG_ON(tag_get(node, 2, offset));
	} else {
		BUG_ON(root_tags_get(root));
	}
	return 0;
}

/*
 * __radix_tree_lookup - lookup an item in a radix tree
 * @root:	radix tree root
 * @index:	index key
 * @nodep:	returns node
 * @slotp:	return slot
 *
 * Lookup and return the item at position @index in the radix
 * tree @root.
 *
 * Until there is more than one item in the tree, no nodes are
 * allocated and @root->rnode is used as a direct slot instead of
 * pointing to a node, in which case *@nodep will be NULL.
 */
void *__radix_tree_lookup(const struct radix_tree_root *root,
			  unsigned long index, struct radix_tree_node **nodep,
			  void ***slotp)
{
	struct radix_tree_node *node, *parent;
	unsigned long maxindex;
	void **slot;

restart:
	parent = NULL;
	slot = (void **)&root->rnode;
	radix_tree_load_root(root, &node, &maxindex);
	if (index > maxindex)
		return NULL;

	while (radix_tree_is_internal_node(node)) {
		unsigned offset;

		if (node == RADIX_TREE_RETRY)
			goto restart;
		parent = entry_to_node(node);
		offset = radix_tree_descend(parent, &node, index);
		slot = parent->slots + offset;
	}
	if (nodep)
		*nodep = parent;
	if (slotp)
		*slotp = slot;
	return node;
}

/*
 * radix_tree_lookup - perform lookup operation on a radix tree
 * @root:	radix tree root
 * @index:	index key
 *
 * Lookup the item at the position @index in the radix tree @root.
 *
 * This function can be called under rcu_read_lock, however the caller
 * must manage lifetimes of leaf nodes (eg. RCU may also be used to free
 * them safely). No RCU barriers are required to access or modify the
 * returned item, however.
 *
 */
void *radix_tree_lookup(const struct radix_tree_root *root, unsigned long index)
{
	return __radix_tree_lookup(root, index, NULL, NULL);
}

static inline void replace_sibling_entries(struct radix_tree_node *node,
				void **slot, int count, int exceptional)
{
}

static void replace_slot(void **slot, void *item,
		struct radix_tree_node *node, int count, int exceptional)
{
	if (WARN_ON_ONCE(radix_tree_is_internal_node(item)))
		return;

	if (node && (count || exceptional)) {
		node->count += count;
		node->exceptional += exceptional;
		replace_sibling_entries(node, slot, count, exceptional);
	}
	*slot = item;
}

static inline void
radix_tree_node_free(struct radix_tree_node *node)
{
	free(node);
}

/*
 * radix_tree_shrink - Shrink radix tree to minimum height
 * @root:	radix tree root
 */
static inline bool radix_tree_shrink(struct radix_tree_root *root,
				radix_tree_update_node_t update_node)
{
	bool shrunk = false;

	for (;;) {
		struct radix_tree_node *node = root->rnode;
		struct radix_tree_node *child;

		if (!radix_tree_is_internal_node(node))
			break;
		node = entry_to_node(node);

		/*
		 * The candidate node has more than one child, or its child
		 * is not at the leftmost slot, or the child is a multiorder
		 * entry, we cannot shrink.
		 */
		if (node->count != 1)
			break;
		child = node->slots[0];
		if (!child)
			break;
		if (!radix_tree_is_internal_node(child) && node->shift)
			break;

		if (radix_tree_is_internal_node(child))
			entry_to_node(child)->parent = NULL;

		/*
		 * We don't need rcu_assign_pointer(), since we are simply
		 * moving the node from one part of the tree to another: if it
		 * was safe to dereference the old pointer to it
		 * (node->slots[0]), it will be safe to dereference the new
		 * one (root->rname) as far as dependent read barriers go.
		 */
		root->rnode = (void *)child;
		if (is_idr(root) && !tag_get(node, IDR_FREE, 0))
			root_tag_clear(root, IDR_FREE);

		/*
		 * We have a dilemma here. The node's slot[0] must not be
		 * NULLed in case there are concurrent lookups expecting to
		 * find the item. However if this was a bottom-level node,
		 * then it may be subject to the slot pointer being visible
		 * to callers dereferencing it. If item corresponding to
		 * slot[0] is subsequently deleted, these callers would expect
		 * their slot to become empty sooner or later.
		 *
		 * For example, lockless pagecache will look up a slot, deref
		 * the page pointer, and if the page has 0 refcount it means it
		 * was concurrently deleted from pagecache so try the deref
		 * again. Fortunately there is already a requirement for logic
		 * to retry the entire slot lookup -- the indirect pointer
		 * problem (replacing direct root node with an indirect pointer
		 * also results in a stale slot). So tag the slot as indirect
		 * to force callers to retry.
		 */
		node->count = 0;
		if (!radix_tree_is_internal_node(child)) {
			node->slots[0] = (void *)RADIX_TREE_RETRY;
			if (update_node)
				update_node(node);
		}

		radix_tree_node_free(node);
		shrunk = true;
	}

	return shrunk;
}

static bool delete_node(struct radix_tree_root *root,
			struct radix_tree_node *node,
			radix_tree_update_node_t update_node)
{
	bool deleted = false;

	do {
		struct radix_tree_node *parent;

		if (node->count) {
			if (node_to_entry(node) == root->rnode)
				deleted |= radix_tree_shrink(root,
								update_node);
			return deleted;
		}

		parent = node->parent;
		if (parent) {
			parent->slots[node->offset] = NULL;
			parent->count--;
		} else {
			/*
			 * Should't the tags already have all been cleared
			 * by the caller?
			 */
			if (!is_idr(root))
				root_tag_clear_all(root);
			root->rnode = NULL;
		}

		radix_tree_node_free(node);
		deleted = true;

		node = parent;
	} while (node);

	return deleted;
}

static bool __radix_tree_delete(struct radix_tree_root *root,
				struct radix_tree_node *node, void **slot)
{
	void *old = *slot;
	int exceptional = radix_tree_exceptional_entry(old) ? -1 : 0;
	unsigned offset = get_slot_offset(node, slot);
	int tag;

	if (is_idr(root))
		node_tag_set(root, node, IDR_FREE, offset);
	else
		for (tag = 0; tag < RADIX_TREE_MAX_TAGS; tag++)
			node_tag_clear(root, node, tag, offset);

	replace_slot(slot, NULL, node, -1, exceptional);
	return node && delete_node(root, node, NULL);
}

/*
 * radix_tree_delete_item - delete an item from a radix tree
 * @root:	radix tree root
 * @index:	index key
 * @item:	expected item
 *
 * Remove @item at @index from the radix tree rooted at @root.
 * 
 * Return: the deleted entry, or %NULL if it was not present
 * of the entry at the given @index was not @item.
 */
void *radix_tree_delete_item(struct radix_tree_root *root,
				unsigned long index, void *item)
{
	struct radix_tree_node *node = NULL;
	void **slot = NULL;
	void *entry;

	entry = __radix_tree_lookup(root, index, &node, &slot);
	if (!slot)
		return NULL;
	if (!entry && (!is_idr(root) || node_tag_get(root, node, IDR_FREE,
						get_slot_offset(node, slot))))
		return NULL;

	if (item && entry != item)
		return NULL;

	__radix_tree_delete(root, node, slot);

	return entry;
}

/*
 * radix_tree_delete - delete an entry from a radix tree
 * @root:	radix tree root
 * @index:	index key
 *
 * Remove the entry at @index from the radix tree rooted at @root
 *
 * Return: The deleted entry, or %NULL if it was not present.
 */
void *radix_tree_delete(struct radix_tree_root *root, unsigned long index)
{
	return radix_tree_delete_item(root, index, NULL);
}

static inline void __set_iter_shift(struct radix_tree_iter *iter,
					unsigned int shift)
{
}

static inline bool
is_sibling_entry(const struct radix_tree_node *parent, void *node)
{
	return false;
}

/*
 * radix_tree_find_next_bit - find the next set bit in a memory region
 *
 * @addr: The address to base the search on
 * @size: The bitmap size in bits
 * @offset: The bitnumber to start searching at
 *
 * Unrollable variant of find_next_bit() for constant size arrays.
 * Tail bits starting from size to roundup(size, BITS_PER_LONG) must be zero.
 * Returns next bit offset, or size if nothing found.
 */
static inline unsigned long
radix_tree_find_next_bit(struct radix_tree_node *node, unsigned int tag,
			 unsigned long offset)
{
	const unsigned long *addr = node->tags[tag];

	if (offset < RADIX_TREE_MAP_SIZE) {
		unsigned long tmp;

		addr += offset / BITS_PER_LONG;
		tmp = *addr >> (offset % BITS_PER_LONG);
		if (tmp)
			return __ffs(tmp) + offset;
		offset = (offset + BITS_PER_LONG) & ~(BITS_PER_LONG - 1);
		while (offset < RADIX_TREE_MAP_SIZE) {
			tmp = *++addr;
			if (tmp)
				return __ffs(tmp) + offset;
		}
	}
	return RADIX_TREE_MAP_SIZE;
}

/* Construct iter->tags bit-mask from node->tags[tag] array */
static void set_iter_tags(struct radix_tree_iter *iter,
				struct radix_tree_node *node, unsigned offset,
				unsigned tag)
{
	unsigned tag_long = offset / BITS_PER_LONG;
	unsigned tag_bit  = offset % BITS_PER_LONG;

	if (!node) {
		iter->tags = 1;
		return;
	}

	iter->tags = node->tags[tag][tag_long] >> tag_bit;

	/* This never happens if RADIX_TREE_TAG_LONGS == 1 */
	if (tag_long < RADIX_TREE_TAG_LONGS - 1) {
		/* Pick tags from next element */
		if (tag_bit)
			iter->tags |= node->tags[tag][tag_long + 1] <<
						(BITS_PER_LONG - tag_bit);
		/* Clip chunk size, here only BITS_PER_LONG tags */
		iter->next_index = __radix_tree_iter_add(iter, BITS_PER_LONG);
	}
}

/*
 * radix_tree_next_chunk - find next chunk of slots for iteration
 *
 * @root:	radix tree root
 * @iter:	iterator state
 * @flags:	RADIX_TREE_ITER_* flags and tag index
 * Returns:	pointer to chunk first slot, or NULL if iteration is over
 */
void **radix_tree_next_chunk(const struct radix_tree_root *root,
			struct radix_tree_iter *iter, unsigned flags)
{
	unsigned tag = flags & RADIX_TREE_ITER_TAG_MASK;
	struct radix_tree_node *node, *child;
	unsigned long index, offset, maxindex;

	if ((flags & RADIX_TREE_ITER_TAGGED) && !root_tag_get(root, tag))
		return NULL;

	/*
	 * Catch next_index overflow after ~0UL. iter->index never overflows
	 * during iterating; it can be zero only at the beginning.
	 * And we cannot overflow iter->next_index in a single step,
	 * because RADIX_TREE_MAP_SHIFT < BITS_PER_LONG.
	 *
	 * This condition also used by radix_tree_next_slot() to stop
	 * contiguous iterating, and forbid switching to the next chunk.
	 */
	index = iter->next_index;
	if (!index && iter->index)
		return NULL;

restart:
	radix_tree_load_root(root, &child, &maxindex);
	if (index > maxindex)
		return NULL;
	if (!child)
		return NULL;

	if (!radix_tree_is_internal_node(child)) {
		/* Single-slot tree */
		iter->index = index;
		iter->next_index = maxindex + 1;
		iter->tags = 1;
		iter->node = NULL;
		__set_iter_shift(iter, 0);
		return (void **)&root->rnode;
	}

	do {
		node = entry_to_node(child);
		offset = radix_tree_descend(node, &child, index);

		if ((flags & RADIX_TREE_ITER_TAGGED) ?
				!tag_get(node, tag, offset) : !child) {
			/* Hole detected */
			if (flags & RADIX_TREE_ITER_CONTIG)
				return NULL;

			if (flags & RADIX_TREE_ITER_TAGGED)
				offset = radix_tree_find_next_bit(node, tag,
						offset + 1);
			else
				while (++offset < RADIX_TREE_MAP_SIZE) {
					void *slot = node->slots[offset];

					if (is_sibling_entry(node, slot))
						continue;
					if (slot)
						break;
				}
			index &= ~node_maxindex(node);
			index += offset << node->shift;
			/* Overflow after ~0UL */
			if (!index)
				return NULL;
			if (offset == RADIX_TREE_MAP_SIZE)
				goto restart;
			child = node->slots[offset];
		}

		if (!child)
			goto restart;
		if (child == RADIX_TREE_RETRY)
			break;
	} while (radix_tree_is_internal_node(child));

	/* Update the iterator state */
	iter->index = (index &~ node_maxindex(node)) | (offset << node->shift);
	iter->next_index = (index | node_maxindex(node)) + 1;
	iter->node = node;
	__set_iter_shift(iter, node->shift);

	if (flags & RADIX_TREE_ITER_TAGGED)
		set_iter_tags(iter, node, offset, tag);

	return node->slots + offset;
}

/*
 * IDR users want to be able to store NULL in the tree, so if the slot isn't
 * free, don't adjust the count, even if it's transitioning between NULL and
 * non-NULL. For the IDA, we mark slots as being IDR_FREE while they still
 * have empty bits, but it only stores NULL in slots when they're being
 * deleted.
 */
static int calculate_count(struct radix_tree_root *root,
				struct radix_tree_node *node, void **slot,
				void *item, void *old)
{
	if (is_idr(root)) {
		unsigned offset = get_slot_offset(node, slot);
		bool free = node_tag_get(root, node, IDR_FREE, offset);

		if (!free)
			return 0;
		if (!old)
			return 1;
	}
	return !!item - !!old;
}

/*
 * __radix_tree_replace		- replace item in a slot
 * @root:		radix tree root
 * @node:		pointer to tree node
 * @slot:		pointer to slot in @node
 * @item:		new item to store in the slot
 * @update_node:	callback for changing leaf nodes.
 *
 * For use with __radix_tree_lookup. Caller must hold tree write locked
 * across slot lookup and replacement.
 */
void __radix_tree_replace(struct radix_tree_root *root,
			  struct radix_tree_node *node,
			  void **slot, void *item,
			  radix_tree_update_node_t update_node)
{
	void *old = *slot;
	int exceptional = !!radix_tree_exceptional_entry(item) -
				!!radix_tree_exceptional_entry(old);
	int count = calculate_count(root, node, slot, item, old);

	/*
	 * This function supports replacing exceptional entries and
	 * deleting entries, but that needs accounting against the
	 * node unless the slot is root->rnode.
	 */
	WARN_ON_ONCE(!node && (slot != (void **)&root->rnode) &&
			(count || exceptional));
	replace_slot(slot, item, node, count, exceptional);

	if (!node)
		return;

	if (update_node)
		update_node(node);

	delete_node(root, node, update_node);
}

/*
 * radix_tree_iter_replace - replace iterm in a slot
 * @root:	radix tree root
 * @slot:	pointer to slot
 * @item:	new item to store in the slot.
 *
 * For use with radix_tree_split() and radix_tree_for_each_slot().
 * Caller must hold tree write locked across split and replacement.
 */
void radix_tree_iter_replace(struct radix_tree_root *root,
				const struct radix_tree_iter *iter,
				void **slot, void *item)
{
	__radix_tree_replace(root, iter->node, slot, item, NULL);
}

static unsigned int iter_offset(const struct radix_tree_iter *iter)
{
	return (iter->index >> iter_shift(iter)) & RADIX_TREE_MAP_MASK;
}

/*
 * radix_tree_iter_tag_clear - clear a tag on the current iterator entry
 * @root:	radix tree root
 * @iter:	iterator state
 * @tag:	tag to clear
 */
void radix_tree_iter_tag_clear(struct radix_tree_root *root,
			const struct radix_tree_iter *iter, unsigned int tag)
{
	node_tag_clear(root, iter->node, tag, iter_offset(iter));
}

/*
 * radix_tree_tagged - test whether any items to the tree are tagged
 * @root:	radix tree root
 * @tag:	tag to test
 */
int radix_tree_tagged(const struct radix_tree_root *root, unsigned int tag)
{
	return root_tag_get(root, tag);
}

static unsigned long next_index(unsigned long index,
				const struct radix_tree_node *node,
				unsigned long offset)
{
	return (index & ~node_maxindex(node)) + (offset << node->shift);
}

void **idr_get_free(struct radix_tree_root *root,
			struct radix_tree_iter *iter, gfp_t gfp,
			unsigned long max)
{
	struct radix_tree_node *node = NULL, *child;
	void **slot = (void **)&root->rnode;
	unsigned long maxindex, start = iter->next_index;
	unsigned int shift, offset = 0;

grow:
	shift = radix_tree_load_root(root, &child, &maxindex);
	if (!radix_tree_tagged(root, IDR_FREE))
		start = max(start, maxindex + 1);
	if (start > max)
		return ERR_PTR(-ENOSPC);

	if (start > maxindex) {
		int error = radix_tree_extend(root, gfp, start, shift);
		
		if (error < 0)
			return ERR_PTR(error);
		shift = error;
		child = root->rnode;
	}

	while (shift) {
		shift -= RADIX_TREE_MAP_SHIFT;
		if (child == NULL) {
			/* Have to add a child node */
			child = radix_tree_node_alloc(gfp, node, root, shift,
							offset, 0, 0);
			if (!child)
				return ERR_PTR(-ENOMEM);
			all_tag_set(child, IDR_FREE);
			*slot = node_to_entry(child);
			if (node)
				node->count++;
		} else if (!radix_tree_is_internal_node(child))
			break;

		node = entry_to_node(child);
		offset = radix_tree_descend(node, &child, start);
		if (!tag_get(node, IDR_FREE, offset)) {
			offset = radix_tree_find_next_bit(node, IDR_FREE,
							offset + 1);
			start = next_index(start, node, offset);
			if (start > max)
				return ERR_PTR(-ENOSPC);
			while (offset == RADIX_TREE_MAP_SIZE) {
				offset = node->offset + 1;
				node = node->parent;
				if (!node)
					goto grow;
				shift = node->shift;
			}
			child = node->slots[offset];
		}
		slot = &node->slots[offset];
	}
	iter->index = start;
	if (node)
		iter->next_index = 1 + min(max, (start | node_maxindex(node)));
	else
		iter->next_index = 1;
	iter->node = node;
	__set_iter_shift(iter, shift);
	set_iter_tags(iter, node, offset, IDR_FREE);

	return slot;
}

/*
 * Load up this CPU's radix_tree_node buffer with sufficient objects to
 * ensure that the addition of a single element in the tree cannot fail.  On
 * success, return zero, with preemption disabled.  On error, return -ENOMEM
 * with preemption not disabled.
 *
 * To make use of this facility, the radix tree must be initialised without
 * __GFP_DIRECT_RECLAIM being passed to INIT_RADIX_TREE().
 */
static int __radix_tree_preload(gfp_t gfp_mask, unsigned nr)
{
	struct radix_tree_preload *rtp;
	struct radix_tree_node *node;
	int ret = -ENOMEM;

	/*
	 * Nodes preloaded by one cgroup can be used by another cgroup, so
	 * they should never be accounted to any particular memory cgroup.
	 */
	gfp_mask &= ~__GFP_ACCOUNT;

	preempt_disable();
	rtp = &radix_tree_preloads;
	while (rtp->nr < nr) {
		preempt_enable();
		node = (struct radix_tree_node *)malloc(
					sizeof(struct radix_tree_node));
		if (node == NULL)
			goto out;
		preempt_disable();
		rtp = &radix_tree_preloads;
		if (rtp->nr < nr) {
			node->parent = rtp->nodes;
			rtp->nodes = node;
			rtp->nr++;
		} else {
			free(node);
		}
	}
	ret = 0;
out:
	return ret;
}

/**
 * idr_preload - preload for idr_alloc()
 * @gfp_mask: allocation mask to use for preloading
 *
 * Preallocate memory to use for the next call to idr_alloc().  This function
 * returns with preemption disabled.  It will be enabled by idr_preload_end().
 */
void idr_preload(gfp_t gfp_mask)
{
	if (__radix_tree_preload(gfp_mask, IDR_PRELOAD_SIZE))
		preempt_disable();
}

/**
 * radix_tree_tag_get - get a tag on a radix tree node
 * @root:               radix tree root
 * @index:              index key
 * @tag:                tag index (< RADIX_TREE_MAX_TAGS)
 *
 * Return values:
 *
 *  0: tag not present or not set
 *  1: tag set
 *
 * Note that the return value of this function may not be relied on, even if
 * the RCU lock is held, unless tag modification and node deletion are excluded
 * from concurrency.
 */
int radix_tree_tag_get(const struct radix_tree_root *root,
			unsigned long index, unsigned int tag)
{
	struct radix_tree_node *node, *parent;
	unsigned long maxindex;

	if (!root_tag_get(root, tag))
		return 0;

	radix_tree_load_root(root, &node, &maxindex);
	if (index > maxindex)
		return 0;

	while (radix_tree_is_internal_node(node)) {
		unsigned offset;

		parent = entry_to_node(node);
		offset = radix_tree_descend(parent, &node, index);

		if (!tag_get(parent, tag, offset))
			return 0;
		if (node == RADIX_TREE_RETRY)
			break;
	}
	return 1;
}





