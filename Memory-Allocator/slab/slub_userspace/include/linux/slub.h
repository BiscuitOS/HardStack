#ifndef _BISCUITOS_SLUB_H
#define _BISCUITOS_SLUB_H

#include "linux/list.h"
#include "linux/gfp.h"
#include "linux/bitmap.h"

typedef unsigned slab_flags_t;
typedef int bool;

enum pageflags {
	PG_slab,
	__NR_PAGEFLAGS,
};

#define ARCH_KMALLOC_MINALIGN	0x40
#define ARCH_SLAB_MINALIGN	8

/*
 * 32 bytes appears to be the most common cache line size,
 * so make that the default here. Architectures with larger
 * cache lines need to provide their own cache.h.
 */
#define L1_CACHE_SHIFT		5
#define L1_CACHE_BYTES		(1 << L1_CACHE_SHIFT)
#define cache_line_size()	L1_CACHE_BYTES

#define ALIGN(x, a)	(((x) + (a) - 1) & ~((a) - 1))

/*
 * Flags to pass to kmem_cache_create().
 * The ones marked DEBUG are only valid if CONFIG_DEBUG_SLUB is set.
 */
/* DEBUG: Poison object */
#define SLAB_POISON		((slab_flags_t)0x00000800U)
/* Align objs on cache lines */
#define SLAB_HWCACHE_ALIGN	((slab_flags_t)0x00002000U)
/* Defer freeing slabs to RCU */
#define SLAB_TYPESAFE_BY_RCU	((slab_flags_t)0x00080000U)
/* Use GFP_DMA memory */
#define SLAB_CACHE_DMA		((slab_flags_t)0x00004000U)
/* DEBUG: Store the last owner for bug hunting */
#define SLAB_STORE_USER		((slab_flags_t)0x00010000U)
/* The following flags affect the page allocator grouping pages by
 * mobility Objects are reclaimable */
#define SLAB_RECLAIM_ACCOUNT	((slab_flags_t)0x00020000U)

#define _RET_IP_	(unsigned long)__builtin_return_address(0)

/*
 * PAGE_ALLOC_COSTLY_ORDER is the order at which allocations are deemed
 * costly to service. That is between allocation orders which should
 * coalesce naturally under reasonable reclaim pressure and thoes which
 * will not.
 */
#define PAGE_ALLOC_COSTLY_ORDER	3

#define OO_SHIFT		16
#define OO_MASK			((1 << OO_SHIFT) - 1)
#define MAX_OBJS_PER_PAGE	32767 /* since page.objects is u15 */

/*
 * Mininum number of partial slabs. These will be left on the partial
 * lists even if they are empty. kmem_cache_shrink may reclaim them.
 */
#define MIN_PARTIAL	5

/*
 * Maximum number of desirable partial slabs.
 * The existence of more partial slabs makes kmem_cache_shrink
 * sort the partial list by the number of objects in use.
 */
#define MAX_PARTIAL	10

#define MAX_NUMNODES	1

/*
 * SLUB directly allocates requests fitting in to an order-1 page
 * (PAGE_SIZE*2). Larger requests are passed to the page allocator.
 */
#define KMALLOC_SHIFT_HIGH	(PAGE_SHIFT + 1)
#define KMALLOC_SHIFT_MAX	(MAX_ORDER + PAGE_SHIFT - 1)
#define KMALLOC_SHIFT_LOW	3

/* Maximum allocation size */
#define KMALLOC_MAX_SIZE	(1UL << KMALLOC_SHIFT_MAX)
/* Maximum size for which we actually use a slab cache */
#define KMALLOC_MAX_CACHE_SIZE	(1UL << KMALLOC_SHIFT_HIGH)
/* Maximum order allocatable via the slab allocagtor */
#define KMALLOC_MAX_ORDER	(KMALLOC_SHIFT_MAX - PAGE_SHIFT)

/*
 * Kmalloc subsystem.
 */
#define KMALLOC_MIN_SIZE	(1 << KMALLOC_SHIFT_LOW)

/*
 * Whenever changing this, take care of that kmalloc_type() and
 * create_kmalloc_caches() still work as intended.
 */
enum kmalloc_cache_type {
	KMALLOC_NORMAL = 0,
	KMALLOC_RECLAIM,
	NR_KMALLOC_TYPES
};

/* A table of kmalloc cache names and sizes */
struct kmalloc_info_struct {
	const char *name;
	unsigned int size;
};

/*
 * State of the slab allocator.
 *
 * This is used to describe the states of the allocator during bootup.
 * Allocators use this to gradually bootstrap themselves. Most allocators
 * have the problem that the structures used for managing slab caches are
 * allocated from slab caches themselves.
 */
enum slab_state {
	DOWN,		/* No slab functionality yet */
	PARTIAL,	/* SLUB: kmem_cache_node available */
	PARTIAL_NODE,	/* SLAB: kmalloc size for node struct available */
	UP,		/* Slab caches usable but not all extras yet */
	FULL		/* Everything is working */
};

struct kmem_cache_cpu {
	void **freelist;	/* Pointer to next available object */
	unsigned long tid;	/* Globally unique transaction id */
	struct page *page;	/* The slab from which we are allocating */
	struct page *partial;	/* Partially allocated fronzen slabs */
};

enum stat_item {
	ALLOC_FASTPATH,		/* Allocation from cup slab */
	ALLOC_SLOWPATH,		/* Allocation by getting a new cpu slab */
	FREE_FASTPATH,		/* Free to cpu slab */
	FREE_SLOWPATH,		/* Freeing not to cpu slab */
	FREE_FROZEN,		/* Freeing to frozen slab */
	FREE_ADD_PARTIAL,	/* Freeing moves slab to partial list  */
	FREE_REMOVE_PARTIAL,	/* Freeing removes last object */
	ALLOC_FROM_PARTIAL,	/* Cpu slab acquired from node partial list */
	ALLOC_SLAB,		/* Cpu slab acquired from page allocator */
	ALLOC_REFILL,		/* Refill cpu slab from slab freelist */
	ALLOC_NODE_MISMATCH,	/* Switching cpu slab */
	FREE_SLAB,		/* Slab freed to the page allocator */
	CPUSLAB_FLUSH,		/* Abandoning of the cpu slab */
	DEACTIVATE_FULL,	/* Cpu slab was full when deactivated */
	DEACTIVATE_EMPTY,	/* Cpu slab was empty when deactivated */
	DEACTIVATE_TO_HEAD,	/* CPU slab was moved to the head of partials */
	DEACTIVATE_TO_TAIL,	/* CPU slab was moved to the tail of partials */
	DEACTIVATE_REMOTE_FREES,/* Slab contained remotely free objects */
	DEACTIVATE_BYPASS,	/* Implicit deactivation */
	ORDER_FALLBACK,		/* Number of times fallback was necessary */
	CMPXCHG_DOUBLE_CPU_FAIL,
	CMPXCHG_DOUBLE_FAIL,
	CPU_PARTIAL_ALLOC,	/* Used cpu partial on alloc */
	CPU_PARTIAL_FREE,	/* Refill cpu partial on free */
	CPU_PARTIAL_NODE,	/* Refill cpu partial from node partial */
	CPU_PARTIAL_DRAIN,	/* Drain cpu partial to node partial */
	NR_SLUB_STAT_ITEMS
};

#define slub_cpu_partial(s)	((s)->cpu_partial)
#define slub_percpu_partial(c)	((c)->partial)
#define slub_set_percpu_partial(c, p)		\
({						\
	slub_percpu_partial(c) = (p)->next;	\
})

/*
 * Word size structure that can be atomically updated or read and that
 * contains both the order and the number of objects that a slab of the
 * given order would contain.
 */
struct kmem_cache_order_objects {
	unsigned int x;
};

/*
 * The slab lists for all objects.
 */
struct kmem_cache_node {
	unsigned long nr_partial;
	struct list_head partial;
};

struct kmem_cache {
	struct kmem_cache_cpu *cpu_slab;
	/* Used for retriving parial slabs etc */
	slab_flags_t flags;
	unsigned long min_partial;
	unsigned int size;	/* The size of an object including meta data */
	unsigned int object_size; /* The size of an object without meta data */
	unsigned int offset;	/* Free pointer offset. */
	
	unsigned inuse;		/* Offset to metadata */
	unsigned int align;	/* Alignment */
	const char *name;	/* Name (only for display) */
	gfp_t allocflags;	/* gfp flags to use on each alloc */
	int refcount;		/* Refcount for slab cache destroy */
	void (*ctor)(void *);

	struct list_head list;	/* List of slab caches */

	struct kmem_cache_order_objects oo;

	/* Allocation and freeing of slabs */
	struct kmem_cache_order_objects max;
	struct kmem_cache_order_objects min;

	unsigned int useroffset;	/* Usercopy region offset */
	unsigned int usersize;		/* Usercopy region size */

	/* Number of per cpu partial objects to keep around */
	unsigned int cpu_partial;

	/* Must locate on tail of struct kmem_cache */
	struct kmem_cache_node *node[MAX_NUMNODES];
};

static inline int PageSlab(struct page *page)
{
	return test_bit(PG_slab, &page->flags);
}

static inline void __SetPageSlab(struct page *page)
{
	__set_bit(PG_slab, &page->flags);
}

static inline void __ClearPageSlab(struct page *page)
{
	__clear_bit(PG_slab, &page->flags);
}

static inline unsigned int order_objects(unsigned int order, unsigned int size)
{
	return ((unsigned int)PAGE_SIZE << order) / size;
}

static inline struct kmem_cache_order_objects oo_make(unsigned int order,
					unsigned int size)
{
	struct kmem_cache_order_objects x = {
		(order << OO_SHIFT) + order_objects(order, size)
	};

	return x;
}

static inline unsigned int oo_order(struct kmem_cache_order_objects x)
{
	return x.x >> OO_SHIFT;
}

static inline unsigned int oo_objects(struct kmem_cache_order_objects x)
{
	return x.x & OO_MASK;
}

static inline unsigned int compound_order(struct page *page)
{
	return 0;
}

extern void *kmem_cache_alloc(struct kmem_cache *s, gfp_t gfpflags);
static inline void *kmem_cache_alloc_node(struct kmem_cache *s, gfp_t flags,
							int node)
{
	return kmem_cache_alloc(s, flags);
}

static inline struct kmem_cache_node *get_node(struct kmem_cache *s, int node)
{
	return s->node[node];
}

static inline void *kmem_cache_zalloc(struct kmem_cache *k, gfp_t flags)
{
	return kmem_cache_alloc(k, flags | __GFP_ZERO);
}

static inline void stat(const struct kmem_cache *s, enum stat_item si)
{
}

#define nr_node_ids	1
#define nr_cpu_ids	4

#define for_each_kmem_cache_node(__s, __node, __n)		\
	for (__node = 0; __node < nr_node_ids; __node++)	\
		if ((__n = get_node(__s, __node)))

extern void *__kmalloc_track_caller(size_t, gfp_t, unsigned long);
#define kmalloc_track_caller(size, flags)	\
	__kmalloc_track_caller(size, flags, _RET_IP_)

static inline enum kmalloc_cache_type kmalloc_type(gfp_t flags)
{
	return flags & __GFP_RECLAIMABLE ? KMALLOC_RECLAIM : KMALLOC_NORMAL;
}

extern void kmem_cache_init(void);

static inline void flush_slab(struct kmem_cache *s, struct kmem_cache_cpu *c);
static void *___slab_alloc(struct kmem_cache *s, gfp_t gfpflags, int node,
				unsigned long addr, struct kmem_cache_cpu *c);
#endif
