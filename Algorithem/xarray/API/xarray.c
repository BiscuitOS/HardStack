/*
 * Xarray.
 *
 * (C) 2019.05.09 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* header of xarray */
#include <linux/xarray.h>
#include <linux/slab.h>

/* node */
struct node {
	char *name;
	unsigned long idx;
};

static struct node node0 = { .name = "IDA", .idx = 0x2344 };

/* Xarray root */
DEFINE_XARRAY(BiscuitOS_root);

static unsigned long xas_max(struct xa_state *xas)
{
	unsigned long max = xas->xa_index;

	return max;
}

static inline bool xa_track_free(const struct xarray *xa)
{
	return xa->xa_flags & XA_FLAGS_TRACK_FREE;
}

static int bs_xas_expand(struct xa_state *xas, void *head)
{
	unsigned long max = xas_max(xas);
	unsigned int shift = 0;

	if (!head) {
		if (max == 0)
			return 0;
		while ((max >> shift) >= XA_CHUNK_SIZE)
			shift += XA_CHUNK_SHIFT;
		return shift + XA_CHUNK_SHIFT;
	} else
		printk("SSDDDD\n");
	return shift;
}

/* Move the radix tree node cache here */
extern struct kmem_cache *radix_tree_node_cachep;

static void *bs_xas_alloc(struct xa_state *xas, unsigned int shift)
{
	struct xa_node *parent = xas->xa_node;
	struct xa_node *node   = xas->xa_alloc;

	if (xas_invalid(xas))
		return NULL;

	if (node) {
		xas->xa_alloc = NULL;
	} else {
		node = kmem_cache_alloc(radix_tree_node_cachep,
				GFP_NOWAIT | __GFP_NOWARN);
		if (!node) {
			xas_set_err(xas, -ENOMEM);
			return NULL;
		}
	}

	if (parent) {
		node->offset = xas->xa_offset;
		parent->count++;
		XA_NODE_BUG_ON(node, parent->count > XA_CHUNK_SIZE);
		printk("AAAAAA\n");
	}
	XA_NODE_BUG_ON(node, shift > BITS_PER_LONG);
	XA_NODE_BUG_ON(node, !list_empty(&node->private_list));
	node->shift = shift;
	node->count = 0;
	node->nr_values = 0;
	RCU_INIT_POINTER(node->parent, xas->xa_node);
	node->array = xas->xa;

	return node;
}

static unsigned int get_offset(unsigned long index, struct xa_node *node)
{
	return (index >> node->shift) & XA_CHUNK_MASK;
}

static void *bs_xas_descend(struct xa_state *xas, struct xa_node *node)
{
	unsigned int offset = get_offset(xas->xa_index, node);
	void *entry = xa_entry(xas->xa, node, offset);

	xas->xa_node = node;
	if (xa_is_sibling(entry)) {
		offset = xa_to_sibling(entry);
		entry  = xa_entry(xas->xa, node, offset);
	}

	xas->xa_offset = offset;
	return entry;
}

static void *bs_xas_create(struct xa_state *xas, bool allow_root)
{
	struct xa_node *node = xas->xa_node;
	struct xarray *xa = xas->xa;
	void *entry;
	int shift;
	void __rcu **slot;
	unsigned int order = xas->xa_shift;

	if (xas_top(node)) {
		entry = xa_head_locked(xa);
		xas->xa_node = NULL;
		shift = bs_xas_expand(xas, entry);
		if (shift < 0)
			return NULL;
		if (!shift && !allow_root)
			shift = XA_CHUNK_SHIFT;
		entry = xa_head_locked(xa);
		slot = &xa->xa_head;
	}

	while (shift > order) {
		shift -= XA_CHUNK_SHIFT;
		if (!entry) {
			node = bs_xas_alloc(xas, shift);
			if (!node)
				break;
			if (xa_track_free(xa))
				printk("AAAAdddd\n");
			rcu_assign_pointer(*slot, xa_mk_node(node));
		}
		entry = bs_xas_descend(xas, node);
		slot = &node->slots[xas->xa_offset];
	}

	return entry;
}

static void xas_update(struct xa_state *xas, struct xa_node *node)
{
	if (xas->xa_update)
		xas->xa_update(node);
	else
		XA_NODE_BUG_ON(node, !list_empty(&node->private_list));
}

static void update_node(struct xa_state *xas, struct xa_node *node,
		int count, int values)
{
	if (!node || (!count && !values))
		return;

	node->count += count;
	node->nr_values += values;
	XA_NODE_BUG_ON(node, node->count > XA_CHUNK_SIZE);
	XA_NODE_BUG_ON(node, node->nr_values > XA_CHUNK_SIZE);
	xas_update(xas, node);
	if (count < 0)
		printk("SB\n");
}

static void *bs_xas_store(struct xa_state *xas, void *entry)
{
	struct xa_node *node;
	void *first, *next;
	unsigned int offset, max;
	void __rcu **slot = &xas->xa->xa_head;
	int values = 0;
	bool value = xa_is_value(entry);
	int count = 0;

	if (entry)
		first = bs_xas_create(xas, !xa_is_node(entry));
	else
		first = xas_load(xas);

	if (xas_invalid(xas))
		return first;
	node = xas->xa_node;
	if (node && (xas->xa_shift < node->shift))
		xas->xa_sibs = 0;
	if ((first == entry) && !xas->xa_sibs)
		return first;

	next = first;
	offset = xas->xa_offset;
	max = xas->xa_offset + xas->xa_sibs;
	if (node) {
		slot = &node->slots[offset];
		if (xas->xa_sibs)
			printk("DSFSDF\n");
	}
	if (!entry)
		xas_init_marks(xas);

	for (;;) {
		rcu_assign_pointer(*slot, entry);
		if (xa_is_node(next))
			printk("FFFFREDDD\n");
		if (!node)
			break;
		count += !next - !entry;
		values += !xa_is_value(first) - !value;
		if (entry) {
			if (offset == max)
				break;
			if (!xa_is_sibling(entry))
				entry = xa_mk_sibling(xas->xa_offset);
		} else {
			if (offset == XA_CHUNK_MASK)
				break;
		}
		next = xa_entry_locked(xas->xa, node, ++offset);
		if (!xa_is_sibling(next)) {
			if (!entry && (offset > max))
				break;
			first = next;
		}
		slot++;
	}
	update_node(xas, node, count, values);
	return first;
}

static void *__bs_xa_store(struct xarray *xa, unsigned long index, void *entry,
		gfp_t gfp)
{
	XA_STATE(xas, xa, index);
	void *curr;

	if (WARN_ON_ONCE(xa_is_advanced(entry)))
		return XA_ERROR(-EINVAL);
	if (xa_track_free(xa) && !entry)
		entry = XA_ZERO_ENTRY;

	curr = bs_xas_store(&xas, entry);

	return NULL;
}

static __init int xarray_demo_init(void)
{
	struct node *np;

	__bs_xa_store(&BiscuitOS_root, node0.idx, &node0, GFP_KERNEL);

	np = xa_load(&BiscuitOS_root, node0.idx);
	
	printk("NP %#lx\n", np->idx);
	
	printk("xarray done......\n");

	return 0;
}
device_initcall(xarray_demo_init);
