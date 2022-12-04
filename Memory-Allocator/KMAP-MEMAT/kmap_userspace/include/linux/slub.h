#ifndef _BISCUITOS_SLUB_H
#define _BISCUITOS_SLUB_H

#include "linux/list.h"
#include "linux/gfp.h"
#include "linux/getorder.h"

typedef unsigned slab_flags_t;
typedef int bool;

#define ENOMEM			12	/* Out of memory */
#define EINVAL			22	/* Invalid argument */

#define ARCH_KMALLOC_MINALIGN	0x40
#define ARCH_SLAB_MINALIGN	8

/*
 * 32 bytes appears to be the most common cache line size,
 * so make that the default here. Architectures with larger
 * cache lines need to provide their own cache.h.
 */
#define L1_CACHE_SHIFT		CONFIG_L1_CACHE_SHIFT
#define L1_CACHE_BYTES		(1 << L1_CACHE_SHIFT)
#define ARCH_DMA_MINALIGN	L1_CACHE_BYTES
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
/* Panic if kmem_cache_create() fails */
#define SLAB_PANIC		((slab_flags_t)0x00040000U)
/* Defer freelist slabs to RCU */
#define SLAB_TYPESAFE_BY_RCU	((slab_flags_t)0x00080000U)
/* Avoid kmemleak tracing */
#define SLAB_NOLEAKTRACE	((slab_flags_t)0x00800000U)
/* DEBUG: Red zone objs on a cache */
#define SLAB_RED_ZONE		((slab_flags_t)0x00000400U)
/* Trace allocations and frees */
#define SLAB_TRACE		((slab_flags_t)0x00200000U)
/* DEBUG: Perform (expensive) checks on alloc/free */
#define SLAB_CONSISTENCY_CHECKS	((slab_flags_t)0x00000100U)
/* Spread some memory over cpuset */
#define SLAB_MEM_SPREAD		((slab_flags_t)0x00100000U)
/* Objects are reclaimable */
#define SLAB_RECLAIM_ACCOUNT	((slab_flags_t)0x00020000U)
#define SLAB_TEMPORARY		SLAB_RECLAIM_ACCOUNT
#define SLAB_ACCOUNT		0
#define SLAB_DEBUG_OBJECTS	0
#define SLAB_DEBUG_FLAGS	0

#define SLAB_CACHE_FLAGS (SLAB_NOLEAKTRACE | SLAB_RECLAIM_ACCOUNT | \
			SLAB_TEMPORARY | SLAB_ACCOUNT)

#define SLAB_CORE_FLAGS (SLAB_HWCACHE_ALIGN | SLAB_CACHE_DMA | SLAB_PANIC | \
	SLAB_TYPESAFE_BY_RCU | SLAB_DEBUG_OBJECTS)

#define CACHE_CREATE_MASK (SLAB_CORE_FLAGS | SLAB_DEBUG_FLAGS | SLAB_CACHE_FLAGS)

/* Common flags permitted for kmem_cache_create */
#define SLAB_FLAGS_PERMITTED	(SLAB_CORE_FLAGS		| \
				 SLAB_RED_ZONE			| \
				 SLAB_POISON			| \
				 SLAB_STORE_USER		| \
				 SLAB_TRACE			| \
				 SLAB_CONSISTENCY_CHECKS	| \
				 SLAB_MEM_SPREAD		| \
				 SLAB_NOLEAKTRACE		| \
				 SLAB_RECLAIM_ACCOUNT		| \
				 SLAB_TEMPORARY			| \
				 SLAB_ACCOUNT)

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
#define KMALLOC_SHIFT_LOW	ilog2(ARCH_DMA_MINALIGN)

/* Maximum allocation size */
#define KMALLOC_MAX_SIZE	(1UL << KMALLOC_SHIFT_MAX)
/* Maximum size for which we actually use a slab cache */
#define KMALLOC_MAX_CACHE_SIZE	(1UL << KMALLOC_SHIFT_HIGH)
/* Maximum order allocatable via the slab allocagtor */
#define KMALLOC_MAX_ORDER	(KMALLOC_SHIFT_MAX - PAGE_SHIFT)

/*
 * Kmalloc subsystem.
 */
#define KMALLOC_MIN_SIZE	(ARCH_DMA_MINALIGN)

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

#define slub_cpu_partial(s)			(0)
#define slub_set_cpu_partial(s, n)
#define slub_percpu_partial(c)			NULL
#define slub_set_percpu_partial(c, p)
#define slub_percpu_partial_read_once(c)	NULL

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

	/* Must locate on tail of struct kmem_cache */
	struct kmem_cache_node *node[MAX_NUMNODES];
};

struct mem_cgroup {
	unsigned long high;
};

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

static inline void *kmem_cache_alloc_trace(struct kmem_cache *s,
			gfp_t flags, size_t size)
{
	void *ret = kmem_cache_alloc(s, flags);
	return ret;
}

extern void *__kmalloc(size_t size, gfp_t flags);
extern struct kmem_cache *
kmalloc_caches[NR_KMALLOC_TYPES][KMALLOC_SHIFT_HIGH + 1];

/*
 * Figure out which kmalloc slab an allocation of a certain size
 * belongs to.
 * 0 = zero alloc
 * 1 =  65 .. 96 bytes
 * 2 = 129 .. 192 bytes
 * n = 2^(n-1)+1 .. 2^n
 */
static inline unsigned int kmalloc_index(size_t size)
{
	if (!size)
		return 0;

	if (size <= KMALLOC_MIN_SIZE)
		return KMALLOC_SHIFT_LOW;

	if (KMALLOC_MIN_SIZE <= 32 && size > 64 && size <= 96)
		return 1;
	if (KMALLOC_MIN_SIZE <= 64 && size > 128 && size <= 192)
		return 0;
	if (size <=          8) return 3;
	if (size <=         16) return 4;
	if (size <=         32) return 5;
	if (size <=         64) return 6;
	if (size <=        128) return 7;
	if (size <=        256) return 8;
	if (size <=        512) return 9;
	if (size <=       1024) return 10;
	if (size <=   2 * 1024) return 11;
	if (size <=   4 * 1024) return 12;
	if (size <=   8 * 1024) return 13;
	if (size <=  16 * 1024) return 14;
	if (size <=  32 * 1024) return 15;
	if (size <=  64 * 1024) return 16;
	if (size <= 128 * 1024) return 17;
	if (size <= 256 * 1024) return 18;
	if (size <= 512 * 1024) return 19;
	if (size <= 1024 * 1024) return 20;
	if (size <= 2 * 1024 * 1024) return 21;
	if (size <= 4 * 1024 * 1024) return 22;
	if (size <= 8 * 1024 * 1024) return 23;
	if (size <= 16 * 1024 * 1024) return 24;
	if (size <= 32 * 1024 * 1024) return 25;
	if (size <= 64 * 1024 * 1024) return 26;

	return -1;
}

/**
 * kmalloc - allocate memory
 * @size: how many bytes of memory are required.
 * @flags: the type of memory to allocate.
 *
 * kmalloc is the normal method of allocating memory
 * for object smaller than page size in the kernel.
 */
static inline void *kmalloc(size_t size, gfp_t flags)
{
	if (__builtin_constant_p(size)) {
		unsigned int index;

		if (size > KMALLOC_MAX_CACHE_SIZE)
			printk("Need Large page\n");

		index = kmalloc_index(size);

		if (!index)
			return ZERO_SIZE_PTR;

		return kmem_cache_alloc_trace(
				kmalloc_caches[kmalloc_type(flags)][index],
				flags, size);
	}
	return __kmalloc(size, flags);
}

/**
 * kzalloc - allocate memory. The memory is set to zero
 * @size: how many bytes of memory are required.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kzalloc(size_t size, gfp_t flags)
{
	return kmalloc(size, flags | __GFP_ZERO);
}

extern void *kmalloc_order(size_t size, gfp_t flags, unsigned int order);

static inline void *kmalloc_order_trace(size_t size, gfp_t flags,
						unsigned int order)
{
	return kmalloc_order(size, flags, order);
}

static inline void *kmalloc_large(size_t size, gfp_t flags)
{
	unsigned int order = get_order(size);
	return kmalloc_order_trace(size, flags, order);
}

static inline struct page *__alloc_pages_node(int nid, gfp_t gfp_mask,
					unsigned int order)
{
	return __alloc_pages(gfp_mask, order);
}

static inline struct page *alloc_pages_node(int nid, gfp_t gfp_mask,
						unsigned int order)
{
	return __alloc_pages_node(nid, gfp_mask, order);
}

#define for_each_memcg_cache(iter, root)	\
	for ((void)(iter), (void)(root); 0; )

#define slab_root_cache		slab_caches
#define root_cache_node		list

#define alloc_pages(gfp_mask, order)	\
			alloc_pages_node(0, gfp_mask, order);

extern void kmem_cache_init(void);

/* The slab cache that manages slab cache information */
extern struct kmem_cache *kmem_cache;

static inline void flush_slab(struct kmem_cache *s, struct kmem_cache_cpu *c);
static void *___slab_alloc(struct kmem_cache *s, gfp_t gfpflags, int node,
				unsigned long addr, struct kmem_cache_cpu *c);
static inline void add_partial(struct kmem_cache_node *n,
				struct page *page, int tail);
static void discard_slab(struct kmem_cache *s, struct page *page);

extern struct kmem_cache *
kmem_cache_create(const char *name, unsigned int size, unsigned int align,
			slab_flags_t flags, void (*ctor)(void *));
void kmem_cache_free(struct kmem_cache *s, void *x);

extern struct kmem_cache *
kmem_cache_create_usercopy(const char *name,
		unsigned int size, unsigned int align,
		slab_flags_t flags,
		unsigned int useroffset, unsigned int usersize,
		void (*ctor)(void *));
extern const char *kstrdup_const(const char *, gfp_t gfp);
extern void kfree_const(const void *x);
extern void kfree(const void *x);
extern void kmem_cache_destroy(struct kmem_cache *s);
extern char *kstrdup(const char *s, gfp_t gfp);
#endif
