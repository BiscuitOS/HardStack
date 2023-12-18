// SPDX-License-Identifier: GPL-2.0
/*
 * MEMORY MMAP: UPDATE GAP
 *
 * (C) 2023.12.17 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/rbtree_types.h>
#include <linux/rbtree_augmented.h>

#define DEV_NAME		"BiscuitOS"
#define BISCUITOS_IO		0xBD
#define MMAP_GAP		_IO(BISCUITOS_IO, 0x00)

static inline unsigned long vma_compute_gap(struct vm_area_struct *vma)
{
	unsigned long gap, prev_end;

	/*
	 * Note: in the rare case of a VM_GROWSDOWN above a VM_GROWSUP, we
	 * allow two stack_guard_gaps between them here, and when choosing
	 * an unmapped area; whereas when expanding we only require one.
	 * That's a little inconsistent, but keeps the code here simpler.
	 */
	gap = vm_start_gap(vma);
	if (vma->vm_prev) {
		prev_end = vm_end_gap(vma->vm_prev);
		if (gap > prev_end)
			gap -= prev_end;
		else
			gap = 0;
	}
	return gap;
}

static inline bool
vma_gap_callbacks_compute_max(struct vm_area_struct *node, bool exit)
{
	struct vm_area_struct *child;
	unsigned long max = vma_compute_gap(node);

	if (node->vm_rb.rb_left) {
		child = rb_entry(node->vm_rb.rb_left,
				struct vm_area_struct, vm_rb);

		if (child->rb_subtree_gap > max)
			max = child->rb_subtree_gap;
	}

	if (node->vm_rb.rb_right) {
		child = rb_entry(node->vm_rb.rb_right,
				struct vm_area_struct, vm_rb);

		if (child->rb_subtree_gap > max) 
			max = child->rb_subtree_gap;
	}

	if (exit && node->rb_subtree_gap == max)
		return true;
	node->rb_subtree_gap = max;
	return false;
}

static inline void
vma_gap_callbacks_propagate(struct rb_node *rb, struct rb_node *stop)
{
	while (rb != stop) {
		struct vm_area_struct *node = rb_entry(rb,
				struct vm_area_struct, vm_rb);

		if (vma_gap_callbacks_compute_max(node, true))
			break;
		rb = rb_parent(&node->vm_rb);
	}
}

static inline void                              
vma_gap_callbacks_copy(struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct vm_area_struct *old = rb_entry(rb_old,
					struct vm_area_struct, vm_rb);
	struct vm_area_struct *new = rb_entry(rb_new,
					struct vm_area_struct, vm_rb);
	new->rb_subtree_gap = old->rb_subtree_gap;
}

static void                                       
vma_gap_callbacks_rotate(struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct vm_area_struct *old = rb_entry(rb_old,
					struct vm_area_struct, vm_rb);
	struct vm_area_struct *new = rb_entry(rb_new,
					struct vm_area_struct, vm_rb);
	new->rb_subtree_gap = old->rb_subtree_gap;
	vma_gap_callbacks_compute_max(old, false);
}

static const struct rb_augment_callbacks vma_gap_callbacks = {
        .propagate = vma_gap_callbacks_propagate,
        .copy = vma_gap_callbacks_copy,
        .rotate = vma_gap_callbacks_rotate
};

static void BiscuitOS_gap(unsigned long addr)
{
	struct vm_area_struct *vma = find_vma(current->mm, addr);

	if (!vma)
		return;

	/* UPDATE GAP */
	vma_gap_callbacks_propagate(&vma->vm_rb, NULL);
}

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case MMAP_GAP:
		BiscuitOS_gap(arg);
		break;
	default:
		break;
	}
	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= BiscuitOS_ioctl,
};

static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

static int __init BiscuitOS_init(void)
{
	misc_register(&BiscuitOS_drv);
	return 0;
}
__initcall(BiscuitOS_init);
