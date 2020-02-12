#ifndef _BISCUITOS_VMALLOC_H
#define _BISCUITOS_VMALLOC_H

#include "linux/buddy.h"
#include "linux/rbtree.h"

struct vm_struct {
	struct vm_struct	*next;
	void			*addr;
	unsigned long		size;
	unsigned long		flags;
	struct page		**page;
	unsigned int		nr_pages;
	phys_addr_t		phys_addr;
	const void		*caller;
};

struct vmap_area {
	unsigned long va_start;
	unsigned long va_end;
	unsigned long flags;
	struct rb_node rb_node;
	struct vm_struct *vm;
};

struct vmap_block_queue {
	struct list_head free;
};

struct vfree_deferred {
	struct llist_head list;
};

extern void vmalloc_init(void);

#endif
