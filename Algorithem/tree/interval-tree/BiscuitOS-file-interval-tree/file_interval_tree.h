#ifndef _FILE_INTERVAL_TREE_H
#define _FILE_INTERVAL_TREE_H

void BiscuitOS_vma_interval_tree_insert(
                struct vm_area_struct *node, struct rb_root_cached *root);

void BiscuitOS_vma_interval_tree_remove(
                struct vm_area_struct *node, struct rb_root_cached *root);

struct vm_area_struct *
BiscuitOS_vma_interval_tree_iter_first(struct rb_root_cached *root,
                        unsigned long start, unsigned long last);

struct vm_area_struct * BiscuitOS_vma_interval_tree_iter_next(
        struct vm_area_struct *node, unsigned long start, unsigned long last);

#define BiscuitOS_vma_interval_tree_foreach(vma, root, start, last)           \
	for (vma = BiscuitOS_vma_interval_tree_iter_first(root, start, last); \
	     vma;                                                             \
	     vma = BiscuitOS_vma_interval_tree_iter_next(vma, start, last))

#endif
