#ifndef _BISCUITOS_VMALLOC_H
#define _BISCUITOS_VMALLOC_H

#include "linux/buddy.h"
#include "linux/rbtree.h"

struct vm_struct {
	struct vm_struct	*next;
	void			*addr;
	unsigned long		size;
	unsigned long		flags;
	struct page		**pages;
	unsigned int		nr_pages;
	phys_addr_t		phys_addr;
	const void		*caller;
};

struct vmap_area {
	unsigned long va_start;
	unsigned long va_end;
	unsigned long flags;
	struct rb_node rb_node;
	struct list_head list;		/* address sorted list */
	struct vm_struct *vm;
};

struct vmap_block_queue {
	struct list_head free;
};

struct vfree_deferred {
	struct llist_head list;
};

/* bits in flags of vmalloc's vm_struct below */
#define VM_IOREMAP		0x00000001	/* ioremap() and firends */
#define VM_ALLOC		0x00000002	/* vmalloc() */
#define VM_MAP			0x00000004	/* vmap()ed pages */
#define VM_USERMAP		0x00000008	/* suitable for remap_vmalloc_range */
#define VM_UNINITIALIZED	0x00000020	/* vm_struct is not fully initialized */
#define VM_NO_GUARD		0x00000040	/* don't add guard page */
#define VM_KASAN		0x00000080	/* has allocated kasan shadow memory */
/* bits [20..32] reserved for arch specific ioremap internals */

extern void *high_memory;
/*
 * Just any arbitrary offset to the start of the vmalloc VM area: the
 * current 8MB value just means that there will be a 8MB "hole" after the
 * physical memory until the kernel virtual memory starts. That means that
 * any out-of-bounds memory accesses will hopefully be caught.
 * The vmalloc() routines leaves a hole of 4KB between each vmalloced
 * area for the same reason. ;)
 */
#define VMALLOC_OFFSET		(8*1024*1024)
#define VMALLOC_START		(((unsigned long)high_memory + \
				VMALLOC_OFFSET) & ~(VMALLOC_OFFSET-1))
#define VMALLOC_END		0xff800000UL

/*
 * Alloc 16MB-aligned ioremap pages
 */
#define IOREMAP_MAX_ORDER	24

static inline size_t get_vm_area_size(const struct vm_struct *area)
{
	if (!(area->flags & VM_NO_GUARD))
		/* return actual size without grard page */
		return area->size - PAGE_SIZE;
	else
		return area->size;
}

static inline bool is_vmalloc_addr(const void *x)
{
	unsigned long addr = (unsigned long)x;

	return addr >= VMALLOC_START && addr < VMALLOC_END;
}

extern void kvfree(const void *addr);
extern void vmalloc_init(void);
extern void *vmalloc(unsigned long size);
extern void vfree(const void *addr);
static void *__vmalloc_node(unsigned long size, unsigned long align,
		gfp_t gfp_mask, pgprot_t prot,
		int node, const void *caller);

#endif
