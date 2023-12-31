// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: FILE-MAPPING REVERSE TREE
 *
 * (C) 2023.12.27 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/rbtree_augmented.h>

static inline unsigned long vma_start_pgoff(struct vm_area_struct *v)
{
	return v->vm_pgoff;
}

static inline unsigned long vma_last_pgoff(struct vm_area_struct *v)
{
	return v->vm_pgoff + vma_pages(v) - 1;
}

/* Callbacks for augmented rbtree insert and remove */
static inline bool vma_interval_tree_augment_compute_max(struct vm_area_struct *node, bool exit)
{
	struct vm_area_struct *child;
	unsigned long max = vma_last_pgoff(node);

	if (node->shared.rb.rb_left) {
		child = rb_entry(node->shared.rb.rb_left,
					struct vm_area_struct, shared.rb);
		if (child->shared.rb_subtree_last > max)
			max = child->shared.rb_subtree_last;
	}

	if (node->shared.rb.rb_right) {
		child = rb_entry(node->shared.rb.rb_right,
					struct vm_area_struct, shared.rb);
		if (child->shared.rb_subtree_last > max)
			max = child->shared.rb_subtree_last;
	}

	if (exit && node->shared.rb_subtree_last == max)
		return true;
	node->shared.rb_subtree_last = max;

	return false;
}

static inline void
vma_interval_tree_augment_propagate(struct rb_node *rb, struct rb_node *stop)
{
	while (rb != stop) {
		struct vm_area_struct *node = rb_entry(rb,
					struct vm_area_struct, shared.rb);

		if (vma_interval_tree_augment_compute_max(node, true))
			break;
		rb = rb_parent(&node->shared.rb);
	}
}

static inline void
vma_interval_tree_augment_copy(struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct vm_area_struct *old = rb_entry(rb_old,
					struct vm_area_struct, shared.rb);
	struct vm_area_struct *new = rb_entry(rb_new,
					struct vm_area_struct, shared.rb);

	new->shared.rb_subtree_last = old->shared.rb_subtree_last;
}

static void                                                       
vma_interval_tree_augment_rotate(struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct vm_area_struct *old = rb_entry(rb_old,
					struct vm_area_struct, shared.rb);
	struct vm_area_struct *new = rb_entry(rb_new,
					struct vm_area_struct, shared.rb);

	new->shared.rb_subtree_last = old->shared.rb_subtree_last;
	vma_interval_tree_augment_compute_max(old, false);
}

static const struct rb_augment_callbacks vma_interval_tree_augment = {
	.propagate = vma_interval_tree_augment_propagate,
	.copy = vma_interval_tree_augment_copy,
	.rotate = vma_interval_tree_augment_rotate
};

/* Insert / remove interval nodes from the tree */
                                                                               
static void __attribute__ ((unused))
BiscuitOS_vma_interval_tree_insert(struct vm_area_struct *node,
				   struct rb_root_cached *root)
{
	struct rb_node **link = &root->rb_root.rb_node, *rb_parent = NULL;
	unsigned long start = vma_start_pgoff(node);
	unsigned long last = vma_last_pgoff(node);
	struct vm_area_struct *parent;
	bool leftmost = true;

	while (*link) {
		rb_parent = *link;
		parent = rb_entry(rb_parent,
				  struct vm_area_struct, shared.rb);

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
		leftmost, &vma_interval_tree_augment);
}

static void __attribute__ ((unused)) BiscuitOS_vma_interval_tree_remove(
	struct vm_area_struct *node, struct rb_root_cached *root)
{
	rb_erase_augmented_cached(&node->shared.rb, root,
					&vma_interval_tree_augment);
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
vma_interval_tree_subtree_search(struct vm_area_struct *node,
				unsigned long start, unsigned long last)
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

static struct vm_area_struct __attribute__ ((unused)) *
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

	return vma_interval_tree_subtree_search(node, start, last);
}

struct vm_area_struct __attribute__ ((unused))*
BiscuitOS_vma_interval_tree_iter_next(struct vm_area_struct *node,
			unsigned long start, unsigned long last)
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
			struct vm_area_struct *right = rb_entry(rb,
					struct vm_area_struct, shared.rb);
			if (start <= right->shared.rb_subtree_last)
				return vma_interval_tree_subtree_search(
							right, start, last);
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
		if (last < vma_start_pgoff(node))
			return NULL;
		else if (start <= vma_last_pgoff(node))
			return node;
		}
}

#define BiscuitOS_vma_interval_tree_foreach(vma, root, start, last)           \
        for (vma = BiscuitOS_vma_interval_tree_iter_first(root, start, last); \
             vma;                                                             \
             vma = BiscuitOS_vma_interval_tree_iter_next(vma, start, last))

static struct address_space mapping;
static struct vm_area_struct vma0, vma1, vma2;
static unsigned long pgoff = 1;

static int __init BiscuitOS_init(void)
{
	struct vm_area_struct *vma;
	struct page *page;

	/* INITIALIZE INTERVAL TREE */
	mapping.i_mmap = RB_ROOT_CACHED;

	/* EMULATE VMA */
	vma0.vm_pgoff = pgoff;
	vma0.vm_start = 0x6000000000;
	vma0.vm_end   = vma0.vm_start + PAGE_SIZE;

	vma1.vm_pgoff = pgoff;
	vma1.vm_start = 0x7000000000;
	vma1.vm_end   = vma1.vm_start + PAGE_SIZE;

	vma2.vm_pgoff = pgoff;
	vma2.vm_start = 0x8000000000;
	vma2.vm_end   = vma2.vm_start + PAGE_SIZE;

	/* INTERVAL VMA */
	BiscuitOS_vma_interval_tree_insert(&vma0, &mapping.i_mmap);
	BiscuitOS_vma_interval_tree_insert(&vma1, &mapping.i_mmap);
	BiscuitOS_vma_interval_tree_insert(&vma2, &mapping.i_mmap);

	/* ALLOC PAGE */
	page = alloc_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;

	/* REVERSE MAPPING */
	page->index = pgoff;
	page->mapping = &mapping; 

	/* TRAVER REVERSE TREE */
	BiscuitOS_vma_interval_tree_foreach(vma, &page->mapping->i_mmap,
					page->index, page->index + 1)
		printk("TRAVER VMA: %#lx\n", vma->vm_start);

	/* RECLAIM */
	BiscuitOS_vma_interval_tree_remove(&vma0, &mapping.i_mmap);
	BiscuitOS_vma_interval_tree_remove(&vma1, &mapping.i_mmap);
	BiscuitOS_vma_interval_tree_remove(&vma2, &mapping.i_mmap);

	return 0;
}

module_init(BiscuitOS_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS MMU Project");
