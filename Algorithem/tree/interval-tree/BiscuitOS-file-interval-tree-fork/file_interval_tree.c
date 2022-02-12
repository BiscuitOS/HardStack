/*
 * File interval tree
 *
 * (C) 2022.02.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rbtree_augmented.h>
#include <linux/mm.h>
#include <linux/rmap.h>
#include "file_interval_tree.h"

static inline unsigned long vma_start_pgoff(struct vm_area_struct *v)
{
	return v->vm_pgoff;
}

static inline unsigned long vma_last_pgoff(struct vm_area_struct *v)
{
	return v->vm_pgoff + vma_pages(v) - 1;
}

/* Callbacks for augmented rbtree insert and remove */

inline unsigned long
BiscuitOS_vma_interval_tree_compute_subtree_last(struct vm_area_struct *node)
{
	unsigned long max = vma_last_pgoff(node), subtree_last;

	if (node->shared.rb.rb_left) {
		subtree_last =
			rb_entry(node->shared.rb.rb_left,
				struct vm_area_struct, 
					shared.rb)->shared.rb_subtree_last;
		if (max < subtree_last)
			max = subtree_last;
	}
	if (node->shared.rb.rb_right) {
		subtree_last =
			rb_entry(node->shared.rb.rb_right,
				struct vm_area_struct,
					shared.rb)->shared.rb_subtree_last;
		if (max < subtree_last)
			max = subtree_last;
	}
	return max;
}

static inline void BiscuitOS_vma_interval_tree_augment_propagate(
				struct rb_node *rb, struct rb_node *stop)
{
	while (rb != stop) {
		struct vm_area_struct *node =
			rb_entry(rb, struct vm_area_struct, shared.rb);
		unsigned long augmented =
			BiscuitOS_vma_interval_tree_compute_subtree_last(node);

		if (node->shared.rb_subtree_last == augmented)
			break;
		node->shared.rb_subtree_last = augmented;
		rb = rb_parent(&node->shared.rb);
	}
}
static inline void BiscuitOS_vma_interval_tree_augment_copy(
			struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct vm_area_struct *old =
			rb_entry(rb_old, struct vm_area_struct, shared.rb);
	struct vm_area_struct *new =
			rb_entry(rb_new, struct vm_area_struct, shared.rb);
	new->shared.rb_subtree_last = old->shared.rb_subtree_last;
}

static void BiscuitOS_vma_interval_tree_augment_rotate(
			struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct vm_area_struct *old =
		rb_entry(rb_old, struct vm_area_struct, shared.rb);
	struct vm_area_struct *new =
		rb_entry(rb_new, struct vm_area_struct, shared.rb);

	new->shared.rb_subtree_last = old->shared.rb_subtree_last;
	old->shared.rb_subtree_last =
		BiscuitOS_vma_interval_tree_compute_subtree_last(old);
}

static const struct rb_augment_callbacks BiscuitOS_vma_interval_tree_augment = {
	.propagate = BiscuitOS_vma_interval_tree_augment_propagate,
	.copy = BiscuitOS_vma_interval_tree_augment_copy,
	.rotate = BiscuitOS_vma_interval_tree_augment_rotate
};

/* Insert / remove interval nodes from the tree */

void BiscuitOS_vma_interval_tree_insert(
		struct vm_area_struct *node, struct rb_root_cached *root)
{
	struct rb_node **link = &root->rb_root.rb_node, *rb_parent = NULL;
	unsigned long start = vma_start_pgoff(node);
	unsigned long last = vma_last_pgoff(node);
	struct vm_area_struct *parent;
	bool leftmost = true;

	while (*link) {
		rb_parent = *link;
		parent = rb_entry(rb_parent, struct vm_area_struct, shared.rb);
		if (parent->shared.rb_subtree_last < last)
			parent->shared.rb_subtree_last = last;
		if (start < vma_start_pgoff(parent))
			link = &parent->shared.rb.rb_left;
		else {
			link = &parent->shared.rb.rb_right;
			leftmost = false;
		}
	}

	node->shared.rb_subtree_last = last;
	rb_link_node(&node->shared.rb, rb_parent, link);
	rb_insert_augmented_cached(&node->shared.rb, root,
			leftmost, &BiscuitOS_vma_interval_tree_augment);
}

void BiscuitOS_vma_interval_tree_remove(
		struct vm_area_struct *node, struct rb_root_cached *root)
{
	rb_erase_augmented_cached(&node->shared.rb,
			root, &BiscuitOS_vma_interval_tree_augment);
}

/*
 * Iterate over intervals intersecting [start;last]
 *
 * Note that a node's interval intersects [start;last] iff:
 *   Cond1: vma_start_pgoff(node) <= last
 * and
 *   Cond2: start <= vma_last_pgoff(node)
 */

static struct vm_area_struct *
BiscuitOS_vma_interval_tree_subtree_search(
	struct vm_area_struct *node, unsigned long start, unsigned long last)
{
	while (true) {
		/*
		 * Loop invariant: start <= node->shared.rb_subtree_last
		 * (Cond2 is satisfied by one of the subtree nodes)
		 */
		if (node->shared.rb.rb_left) {
			struct vm_area_struct *left =
				rb_entry(node->shared.rb.rb_left,
					struct vm_area_struct, shared.rb);
			if (start <= left->shared.rb_subtree_last) {
				/*
				 * Some nodes in left subtree satisfy Cond2.
				 * Iterate to find the leftmost such node N.
				 * If it also satisfies Cond1, that's the
				 * match we are looking for. Otherwise, there
				 * is no matching interval as nodes to the
				 * right of N can't satisfy Cond1 either.
				 */
				node = left;
				continue;
			}
		}
		if (vma_start_pgoff(node) <= last) {            /* Cond1 */
			if (start <= vma_last_pgoff(node))      /* Cond2 */
				return node;    /* node is leftmost match */
			if (node->shared.rb.rb_right) {
				node = rb_entry(node->shared.rb.rb_right,
					struct vm_area_struct, shared.rb);
				if (start <= node->shared.rb_subtree_last)
					continue;
			}
		}
		return NULL;    /* No match */
	}
}

struct vm_area_struct *
BiscuitOS_vma_interval_tree_iter_first(struct rb_root_cached *root,
                        unsigned long start, unsigned long last)
{
	struct vm_area_struct *node, *leftmost;

	if (!root->rb_root.rb_node)
		return NULL;

	/*
	 * Fastpath range intersection/overlap between A: [a0, a1] and
	 * B: [b0, b1] is given by:
	 *
	 *         a0 <= b1 && b0 <= a1
	 *
	 *  ... where A holds the lock range and B holds the smallest
	 * 'start' and largest 'last' in the tree. For the later, we
	 * rely on the root node, which by augmented interval tree
	 * property, holds the largest value in its last-in-subtree.
	 * This allows mitigating some of the tree walk overhead for
	 * for non-intersecting ranges, maintained and consulted in O(1).
	 */
	node = rb_entry(root->rb_root.rb_node,
			struct vm_area_struct, shared.rb);
	if (node->shared.rb_subtree_last < start)
		return NULL;

	leftmost = rb_entry(root->rb_leftmost,
				struct vm_area_struct, shared.rb);
	if (vma_start_pgoff(leftmost) > last)
		return NULL;

	return BiscuitOS_vma_interval_tree_subtree_search(node, start, last);
}

struct vm_area_struct * BiscuitOS_vma_interval_tree_iter_next(
	struct vm_area_struct *node, unsigned long start, unsigned long last)
{
	struct rb_node *rb = node->shared.rb.rb_right, *prev;

	while (true) {
		/*
		 * Loop invariants:
		 *   Cond1: vma_start_pgoff(node) <= last
		 *   rb == node->shared.rb.rb_right
		 *
		 * First, search right subtree if suitable
		 */
		if (rb) {
			struct vm_area_struct *right =
				rb_entry(rb, struct vm_area_struct, shared.rb);
			if (start <= right->shared.rb_subtree_last)
				return BiscuitOS_vma_interval_tree_subtree_search(right, start, last);
		}

		/* Move up the tree until we come from a node's left child */
		do {
			rb = rb_parent(&node->shared.rb);
			if (!rb)
				return NULL;
			prev = &node->shared.rb;
			node = rb_entry(rb, struct vm_area_struct, shared.rb);
			rb = node->shared.rb.rb_right;
		} while (prev == rb);

		/* Check if the node intersects [start;last] */
		if (last < vma_start_pgoff(node))               /* !Cond1 */
			return NULL;
		else if (start <= vma_last_pgoff(node))         /* Cond2 */
			return node;
	}                                
}

/* Insert node immediately after prev in the interval tree */
void vma_interval_tree_insert_after(struct vm_area_struct *node,
                                    struct vm_area_struct *prev,
                                    struct rb_root_cached *root)
{
	struct rb_node **link;
	struct vm_area_struct *parent;
	unsigned long last = vma_last_pgoff(node);

	VM_BUG_ON_VMA(vma_start_pgoff(node) != vma_start_pgoff(prev), node);

	if (!prev->shared.rb.rb_right) {
		parent = prev;
		link = &prev->shared.rb.rb_right;
	} else {
		parent = rb_entry(prev->shared.rb.rb_right,
				struct vm_area_struct, shared.rb);
		if (parent->shared.rb_subtree_last < last)
			parent->shared.rb_subtree_last = last;
		while (parent->shared.rb.rb_left) {
			parent = rb_entry(parent->shared.rb.rb_left,
					struct vm_area_struct, shared.rb);
			if (parent->shared.rb_subtree_last < last)
				parent->shared.rb_subtree_last = last;
		}
		link = &parent->shared.rb.rb_left;
	}

	node->shared.rb_subtree_last = last;
	rb_link_node(&node->shared.rb, &parent->shared.rb, link);
	rb_insert_augmented(&node->shared.rb, &root->rb_root,
				&BiscuitOS_vma_interval_tree_augment);
}
