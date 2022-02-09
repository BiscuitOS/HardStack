/*
 * Anonymous interval tree
 *
 * (C) 2022.02.09 BuddyZhang1 <buddy.zhang@aliyun.com>
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

static inline unsigned long vma_start_pgoff(struct vm_area_struct *v)
{
	return v->vm_pgoff;
}

static inline unsigned long vma_last_pgoff(struct vm_area_struct *v)
{
	return v->vm_pgoff + vma_pages(v) - 1;
}

static inline unsigned long avc_start_pgoff(struct anon_vma_chain *avc)
{
	return vma_start_pgoff(avc->vma);
}

static inline unsigned long avc_last_pgoff(struct anon_vma_chain *avc)
{
	return vma_last_pgoff(avc->vma);
}

/* Callbacks for augmented rbtree insert and remove */

static inline unsigned long 
BiscuitOS__anon_vma_interval_tree_compute_subtree_last(
					struct anon_vma_chain *node)        
{
	unsigned long max = avc_last_pgoff(node), subtree_last;

	if (node->rb.rb_left) {
		subtree_last = rb_entry(node->rb.rb_left,
				struct anon_vma_chain, rb)->rb_subtree_last;
		if (max < subtree_last)
			max = subtree_last;
	}

	if (node->rb.rb_right) {
		subtree_last = rb_entry(node->rb.rb_right,
				struct anon_vma_chain, rb)->rb_subtree_last;
		if (max < subtree_last)
			max = subtree_last;
	}
	return max;
}
                                                                              
static inline void
BiscuitOSBiscuitOS__anon_vma_interval_tree_augment_propagate(
				struct rb_node *rb, struct rb_node *stop)
{
	while (rb != stop) {                                            
		struct anon_vma_chain *node = 
			rb_entry(rb, struct anon_vma_chain, rb);       
		unsigned long augmented;

		augmented = 
		BiscuitOS__anon_vma_interval_tree_compute_subtree_last(node);
		if (node->rb_subtree_last == augmented)                     
			break;                                          
		node->rb_subtree_last = augmented;                          
		rb = rb_parent(&node->rb);                         
	}                                                               
}

static inline void BiscuitOSBiscuitOS__anon_vma_interval_tree_augment_copy(
			struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct anon_vma_chain *old = 
			rb_entry(rb_old, struct anon_vma_chain, rb);
	struct anon_vma_chain *new =
			rb_entry(rb_new, struct anon_vma_chain, rb);

	new->rb_subtree_last = old->rb_subtree_last;
}

static void BiscuitOSBiscuitOS__anon_vma_interval_tree_augment_rotate(
			struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct anon_vma_chain *old =
			rb_entry(rb_old, struct anon_vma_chain, rb);
	struct anon_vma_chain *new =
			rb_entry(rb_new, struct anon_vma_chain, rb);

	new->rb_subtree_last = old->rb_subtree_last;
	old->rb_subtree_last =
		BiscuitOS__anon_vma_interval_tree_compute_subtree_last(old);
}

static const struct rb_augment_callbacks BiscuitOS__anon_vma_interval_tree_augment = {
	.propagate = BiscuitOSBiscuitOS__anon_vma_interval_tree_augment_propagate,
	.copy = BiscuitOSBiscuitOS__anon_vma_interval_tree_augment_copy,
	.rotate = BiscuitOSBiscuitOS__anon_vma_interval_tree_augment_rotate
};

/* Insert / remove interval nodes from the tree */                            

static inline void BiscuitOS__anon_vma_interval_tree_insert(
		struct anon_vma_chain *node, struct rb_root_cached *root)
{
	struct rb_node **link = &root->rb_root.rb_node, *rb_parent = NULL;
	unsigned long start = avc_start_pgoff(node), last = avc_last_pgoff(node);
	struct anon_vma_chain *parent;
	bool leftmost = true; 

	while (*link) {
		rb_parent = *link;
		parent = rb_entry(rb_parent, struct anon_vma_chain, rb);
		if (parent->rb_subtree_last < last)
			parent->rb_subtree_last = last;
		if (start < avc_start_pgoff(parent))
			link = &parent->rb.rb_left;
		else {
			link = &parent->rb.rb_right;
			leftmost = false;
		}
	}

	node->rb_subtree_last = last;
	rb_link_node(&node->rb, rb_parent, link);
	rb_insert_augmented_cached(&node->rb, root,
		leftmost, &BiscuitOS__anon_vma_interval_tree_augment);
}

static inline void BiscuitOS__anon_vma_interval_tree_remove(
		struct anon_vma_chain *node, struct rb_root_cached *root)                
{
	rb_erase_augmented_cached(&node->rb, 
			root, &BiscuitOS__anon_vma_interval_tree_augment);
}

/*                                                                            
 * Iterate over intervals intersecting [start;last]                           
 *                                                                            
 * Note that a node's interval intersects [start;last] iff:                   
 *   Cond1: avc_start_pgoff(node) <= last                                             
 * and                                                                        
 *   Cond2: start <= avc_last_pgoff(node)                                             
 */                                                                           
                                                                              
static struct anon_vma_chain *
BiscuitOS__anon_vma_interval_tree_subtree_search(
	struct anon_vma_chain *node, unsigned long start, unsigned long last)
{                                                                             
	while (true) {
		/*
		 * Loop invariant: start <= node->rb_subtree_last
		 * (Cond2 is satisfied by one of the subtree nodes)
		 */
		if (node->rb.rb_left) {
			struct anon_vma_chain *left = rb_entry(node->rb.rb_left,
			struct anon_vma_chain, rb);

			if (start <= left->rb_subtree_last) {
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
		if (avc_start_pgoff(node) <= last) {	/* Cond1 */
			if (start <= avc_last_pgoff(node))	/* Cond2 */
				return node;    /* node is leftmost match */
			if (node->rb.rb_right) {
				node = rb_entry(node->rb.rb_right,
						struct anon_vma_chain, rb);
				if (start <= node->rb_subtree_last)
					continue;
			}
		}
		return NULL;    /* No match */
	}
}
                                                                              
static inline struct anon_vma_chain *
BiscuitOS__anon_vma_interval_tree_iter_first(
	struct rb_root_cached *root, unsigned long start, unsigned long last)
{                                                                             
	struct anon_vma_chain *node, *leftmost;

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
	node = rb_entry(root->rb_root.rb_node, struct anon_vma_chain, rb);
	if (node->rb_subtree_last < start)
		return NULL;

	leftmost = rb_entry(root->rb_leftmost, struct anon_vma_chain, rb);
	if (avc_start_pgoff(leftmost) > last)
		return NULL;
	return BiscuitOS__anon_vma_interval_tree_subtree_search(
						node, start, last);
}

static inline struct anon_vma_chain *
BiscuitOS__anon_vma_interval_tree_iter_next(
	struct anon_vma_chain *node, unsigned long start, unsigned long last)
{
	struct rb_node *rb = node->rb.rb_right, *prev;

	while (true) {
		/*
		 * Loop invariants:
		 *   Cond1: avc_start_pgoff(node) <= last
		 *   rb == node->rb.rb_right
		 *
		 * First, search right subtree if suitable
		 */
		if (rb) {
			struct anon_vma_chain *right = 
				rb_entry(rb, struct anon_vma_chain, rb);
			if (start <= right->rb_subtree_last)
				return BiscuitOS__anon_vma_interval_tree_subtree_search(right, start, last); 
		}

		/* Move up the tree until we come from a node's left child */
		do {
			rb = rb_parent(&node->rb);
			if (!rb)
				return NULL;
			prev = &node->rb;
			node = rb_entry(rb, struct anon_vma_chain, rb);
			rb = node->rb.rb_right;
		} while (prev == rb);

		/* Check if the node intersects [start;last] */
		if (last < avc_start_pgoff(node))               /* !Cond1 */
			return NULL;
		else if (start <= avc_last_pgoff(node))         /* Cond2 */
			return node;
	}
}

void BiscuitOS_anon_vma_interval_tree_insert(struct anon_vma_chain *node,
					     struct rb_root_cached *root)
{
	printk("FFFFF\n");
	BiscuitOS__anon_vma_interval_tree_insert(node, root);
	printk("ZZZZZ\n");
}

struct anon_vma_chain *
BiscuitOS_anon_vma_interval_tree_iter_first(struct rb_root_cached *root,
                                  unsigned long first, unsigned long last)
{
	return BiscuitOS__anon_vma_interval_tree_iter_first(root, first, last);
}

struct anon_vma_chain *
BiscuitOS_anon_vma_interval_tree_iter_next(struct anon_vma_chain *node,
                                 unsigned long first, unsigned long last)
{
	return BiscuitOS__anon_vma_interval_tree_iter_next(node, first, last);
}

void BiscuitOS_anon_vma_interval_tree_remove(struct anon_vma_chain *node,
                                   struct rb_root_cached *root)
{
	BiscuitOS__anon_vma_interval_tree_remove(node, root);
}
