/*
 * Slub Memory Allocator
 *
 * (C) 2020.02.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "linux/buddy.h"
#include "linux/slub.h"
#include "linux/getorder.h"

static struct kmem_cache *kmem_cache_node;
struct kmem_cache *kmem_cache;
static unsigned int slub_min_objects;
static unsigned int slub_min_order;
static unsigned int slub_max_order = PAGE_ALLOC_COSTLY_ORDER;
enum slab_state slab_state;
gfp_t gfp_allowed_mask = GFP_BOOT_MASK;
LIST_HEAD(slab_caches);

/*
 * Figure out what the alignment of the objects will be given a set of
 * flags, a user specified alignment and the size of the object.
 */
static unsigned int calculate_alignment(slab_flags_t flags,
		unsigned int align, unsigned int size)
{
	/*
	 * If the user wants hardware cache aligned objects then follow that
	 * suggestion if the object is sufficiently large.
	 *
	 * The hardware cache alignment cannot override the specified
	 * alignment though. If that is greater then use it.
	 */
	if (flags & SLAB_HWCACHE_ALIGN) {
		unsigned int ralign;

		ralign = cache_line_size();
		while (size <= ralign / 2)
			ralign /= 2;
		align = max(align, ralign);
	}

	if (align < ARCH_SLAB_MINALIGN)
		align = ARCH_SLAB_MINALIGN;

	return ALIGN(align, sizeof(void *));
}

static slab_flags_t kmem_cache_flags(unsigned int object_size,
		slab_flags_t flags, const char *name,
		void (*ctor)(void *))
{
	return flags;
}

/*
 * Calculate the order of allocation given an slab objects size.
 *
 * The order of allocation has significant impact on performance and other
 * system components. Generally order 0 allocation should be preferred since
 * order 0 does not cause fragmentation in the page allocator. Larger objects
 * be problematic to put into order 0 slabs because there may be too much
 * unused space left. We go to a higher order if more than 1/16 of the slab
 * would be wasted.
 *
 * In order to reach satisfactory performance we must ensure that a minimum
 * number of objects is in one slab. Otherwise we may generate too much
 * activity on the partial lists which requires taking the list_lock. This is
 * less a concern for large slabs though which are rarely used.
 *
 * slab_max_order specifies the order where we begin to stop considering the
 * number of objects in a slab as critical. If we reach slub_max_order then
 * we try to keep the page order as low as possible. So we accept more waste
 *
 * Higher order allocations also allow the placement of more objects in a
 * slab and thereby reduce object handling overhead. If the user has
 * requested a higher mininum order then we start with that one instead of
 * the smallest order which will fit the object.
 */
static inline unsigned int slab_order(unsigned int size,
		unsigned int min_objects, unsigned int max_order,
		unsigned int fract_leftover)
{
	unsigned int min_order = slub_min_order;
	unsigned int order;

	if (order_objects(min_order, size) > MAX_OBJS_PER_PAGE)
		return get_order(size * MAX_OBJS_PER_PAGE) - 1;

	for (order = max(min_order, 
			(unsigned int)get_order(min_objects * size));
			order <= max_order; order++) {
		unsigned int slab_size = (unsigned int)PAGE_SIZE << order;
		unsigned int rem;

		rem = slab_size % size;

		if (rem <= slab_size / fract_leftover)
			break;
	}

	return order;
}

static inline int calculate_order(unsigned int size)
{
	unsigned int order;
	unsigned int min_objects;
	unsigned int max_objects;

	/*
	 * Attempt to find best configuration for a slab. This
	 * works by first attempting to generate a layout with
	 * the best configuration and backing off gradually.
	 *
	 * First we increase the acceptable waste in a slab. Then
	 * we reduce the minimum objects required in a slab.
	 */
	min_objects = slub_min_objects;
	if (!min_objects)
		min_objects = 4 * (fls(nr_cpu_ids) + 1);
	max_objects = order_objects(slub_max_order, size);
	min_objects = min(min_objects, max_objects);

	while (min_objects > 1) {
		unsigned int fraction;

		fraction = 16;
		while (fraction >= 4) {
			order = slab_order(size, min_objects,
					slub_max_order, fraction);
			if (order <= slub_max_order)
				return order;
			fraction /= 2;
		}
		min_objects--;
	}

	/*
	 * We were unable to place multiple objects in a slab. Now
	 * lets see if we can place a single object there.
	 */
	order = slab_order(size, 1, slub_max_order, 1);
	if (order <= slub_max_order)
		return order;

	/*
	 * Doh this slab cannot be placed using slub_max_order.
	 */
	order = slab_order(size, 1, MAX_ORDER, 1);
	if (order < MAX_ORDER)
		return order;
	return -1;
}

/*
 * calculate_sizes() determines the order and the distribution of data within
 * a slab object.
 */
static int calculate_sizes(struct kmem_cache *s, int forced_order)
{
	slab_flags_t flags = s->flags;
	unsigned int size = s->object_size;
	unsigned int order;

	/*
	 * Round up object size to the next word boundary. We can only
	 * place the free pointer at word boundaries and this determines
	 * the possible location of the free pointer.
	 */
	size = ALIGN(size, sizeof(void *));

	/*
	 * With that we have determined the number of bytes in actual use
	 * by the object. This is the potential offset to the free pointer.
	 */
	s->inuse = size;

	if (((flags & (SLAB_TYPESAFE_BY_RCU | SLAB_POISON)) || s->ctor)) {
		/*
		 * Relocate free pointer after the object if it is not
		 * permitted to overwrite the first word of the object on
		 * kmem_cache_free.
		 *
		 * This is the case if we do RCU, have a constructor or
		 * destructor or are poisoning the objects.
		 */
		s->offset = size;
		size += sizeof(void *);
	}

	/*
	 * SLUB stores one object immediately after another beginning from
	 * offset 0. In order to align the objects we have to simply size
	 * each object to conform to the alignment.
	 */
	size = ALIGN(size, s->align);
	s->size = size;
	if (forced_order >= 0)
		order = forced_order;
	else
		order = calculate_order(size);

	if ((int)order < 0)
		return 0;

	s->allocflags = 0;
	if (order)
		s->allocflags |= __GFP_COMP;

	if (s->flags & SLAB_CACHE_DMA)
		s->allocflags |= GFP_DMA;

	if (s->flags & SLAB_RECLAIM_ACCOUNT)
		s->allocflags |= __GFP_RECLAIMABLE;

	/*
	 * Determine the number of objects per slab
	 */
	s->oo = oo_make(order, size);
	s->min = oo_make(get_order(size), size);
	if (oo_objects(s->oo) >> oo_objects(s->max))
		s->max = s->oo;

	return !!oo_objects(s->oo);
}

static void set_min_partial(struct kmem_cache *s, unsigned long min)
{
	if (min < MIN_PARTIAL)
		min = MIN_PARTIAL;
	else if (min > MAX_PARTIAL)
		min = MAX_PARTIAL;
	s->min_partial = min;
}

static inline int kmem_cache_has_cpu_partial(struct kmem_cache *s)
{
	return 1;
}

static void set_cpu_partial(struct kmem_cache *s)
{
	/*
	 * cpu_partial determined the maximum number of objects kept in the
	 * per cpu partial lists of a processor.
	 *
	 * Per cpu partial lists mainly contain slabs that just have one
	 * object freed. If they are used for allocation then they can be
	 * filled up again with minimal effort. The slab will never hit the
	 * per node partial lists and therefore no locking will be required
	 *
	 * This setting also determines
	 *
	 * A) The number of objects from per cpu partial slabs dumped to the
	 *    per node list when we reach the limit.
	 * B) The number of objects in cpu partial slabs to extract from the
	 *    per node list when we run out of per cpu objects. We only fetch
	 *    50% to keep some capacity around for frees.
	 */
	if (!kmem_cache_has_cpu_partial(s))
		s->cpu_partial = 0;
	else if (s->size >= PAGE_SIZE)
		s->cpu_partial = 2;
	else if (s->size >= 1024)
		s->cpu_partial = 6;
	else if (s->size >= 256)
		s->cpu_partial = 13;
	else
		s->cpu_partial = 30;
}

/*
 * Slab allocation and freeing
 */
static inline struct page *alloc_slab_page(struct kmem_cache *s,
		gfp_t flags, int node, struct kmem_cache_order_objects oo)
{
	struct page *page;
	unsigned int order = oo_order(oo);

	page = __alloc_pages(flags, order);
	return page;
}

static inline void *freelist_ptr(const struct kmem_cache *s, void *ptr,
					unsigned long ptr_addr)
{
	return ptr;
}

static inline void set_freepointer(struct kmem_cache *s, void *object, void *fp)
{
	unsigned long freeptr_addr = (unsigned long)object + s->offset;

	*(void **)freeptr_addr = freelist_ptr(s, fp, freeptr_addr);
}

/* Return the freelist pointer recorded at location ptr_addr */
static inline void *freelist_dereference(const struct kmem_cache *s,
					void *ptr_addr)
{
	return freelist_ptr(s, (void *)*(unsigned long *)(ptr_addr),
					(unsigned long)ptr_addr);
}

static inline void *get_freepointer(struct kmem_cache *s, void *object)
{
	return freelist_dereference(s, object + s->offset);
}

static inline void *get_freepointer_safe(struct kmem_cache *s, void *object)
{
	unsigned long freepointer_addr;
	void *p;

	freepointer_addr = (unsigned long)object + s->offset;
	return freelist_ptr(s, p, freepointer_addr);
}

static struct page *allocate_slab(struct kmem_cache *s, gfp_t flags, int node)
{
	struct page *page;
	struct kmem_cache_order_objects oo = s->oo;
	gfp_t alloc_gfp;
	void *start, *p, *next;
	int idx, order;
	bool shuffle;

	flags &= gfp_allowed_mask;

	flags |= s->allocflags;

	/*
	 * Let the initial higher-order allocation fail under memory pressure
	 * so we fall-back to the minimum order allocation.
	 */
	alloc_gfp = (flags | __GFP_NOWARN | __GFP_NORETRY) & ~__GFP_NOFAIL;
	if ((alloc_gfp & __GFP_DIRECT_RECLAIM) && 
				oo_order(oo) > oo_order(s->min))
		alloc_gfp = (alloc_gfp | __GFP_NOMEMALLOC) & 
						~(__GFP_RECLAIM|__GFP_NOFAIL);
	page = alloc_slab_page(s, alloc_gfp, node, oo);
	if (unlikely(!page)) {
		oo = s->min;
		alloc_gfp = flags;

		/*
		 * Allocation may have failed due to fragmentation
		 * Try a lower order alloc if possible.
		 */
		page = alloc_slab_page(s, alloc_gfp, node, oo);
		if (unlikely(!page))
			goto out;
		stat(s, ORDER_FALLBACK);
	}

	page->objects = oo_objects(oo);

	order = compound_order(page);
	page->slab_cache = s;
	__SetPageSlab(page);

	start = page_address(page);
	page->freelist = start;
	for (idx = 0, p = start; idx < page->objects - 1; idx++) {
		next = p + s->size;
		set_freepointer(s, p, next);
		p = next;
	}
	set_freepointer(s, p, NULL);

	page->inuse = page->objects;
	page->frozen = 1;
out:
	if (!page)
		return NULL;

	return page;
}

static struct page *new_slab(struct kmem_cache *s, gfp_t flags, int node)
{
	return allocate_slab(s, 
		flags & (GFP_RECLAIM_MASK | GFP_CONSTRAINT_MASK), node);
}

static void init_kmem_cache_node(struct kmem_cache_node *n)
{
	n->nr_partial = 0;
	INIT_LIST_HEAD(&n->partial);
}

/*
 * Management of partially allocated slabs.
 */
static inline void
__add_partial(struct kmem_cache_node *n, struct page *page, int tail)
{
	n->nr_partial++;
	if (tail == DEACTIVATE_TO_TAIL)
		list_add_tail(&page->lru, &n->partial);
	else
		list_add(&page->lru, &n->partial);
}

/*
 * No kmalloc_node yet so do it by hand. We know that this is the first
 * slab on the node for this slabcache. There area no concurrent accesses
 * possible.
 *
 * Note that this function only works on the kmem_cache_node
 * when allocating for the kmem_cache_node. This is used for bootstrapping
 * memory on a fresh node that has no slab structures yet.
 */
static void early_kmem_cache_node_alloc(int node)
{
	struct page *page;
	struct kmem_cache_node *n;

	page = new_slab(kmem_cache_node, GFP_NOWAIT, node);
	if (!page) {
		printk("SLUB: Unable to allocate memory from node 0\n");
		printk("SLUB: Allocating a useless per node structure "
			     "in order to be able to continue.\n");
	}

	n = page->freelist;
	page->freelist = get_freepointer(kmem_cache_node, n);
	page->inuse = 1;
	page->frozen = 0;
	kmem_cache_node->node[node] = n;
	init_kmem_cache_node(n);

	/*
	 * No locks need to be taken here as it has just been
	 * initialized and there is no concurrent access.
	 */
	__add_partial(n, page, DEACTIVATE_TO_HEAD);
}

/*
 * Another one that disable interrupt and compensates for possible
 * cpu changes by refetching the per cpu area pointer.
 */
static void *__slab_alloc(struct kmem_cache *s, gfp_t gfpflags, int node,
				unsigned long addr, struct kmem_cache_cpu *c)
{
	void *p;
	unsigned long flags;

	c = s->cpu_slab;

	p = ___slab_alloc(s, gfpflags, node, addr, c);
	return p;
}

/* Interrupts must be disabled (for the fallback code to work right) */
static inline bool __cmpxchg_double_slab(struct kmem_cache *s,
		struct page *page, void *freelist_old,
		unsigned long counters_old, void *freelist_new,
		unsigned long counters_new, const char *n)
{
	if (page->freelist == freelist_old &&
				page->counters == counters_old) {
		page->freelist = freelist_new;
		page->counters = counters_new;
		return 1;
	}

	stat(s, CMPXCHG_DOUBLE_FAIL);
	return 0;
}

static inline void remove_partial(struct kmem_cache_node *n, struct page *page)
{
	list_del(&page->lru);
	n->nr_partial--;
}

/*
 * Remove slab from the partial list, freeze it and
 * return the pointer to the freelist.
 *
 * Returns a list of objects or NULL if it fails.
 */
static inline void *acquire_slab(struct kmem_cache *s,
		struct kmem_cache_node *n, struct page *page,
		int mode, int *objects)
{
	void *freelist;
	unsigned long counters;
	struct page new;

	/*
	 * default page:
	 * 32       31                  16                          0
	 * +--------+-------------------+---------------------------+
	 * | frozen |      objects      |           inuse           |
	 * +--------+-------------------+---------------------------+
	 * |                        counters                        |
	 * +--------------------------------------------------------+
	 */
	freelist = page->freelist;
	counters = page->counters;
	new.counters = counters;
	*objects = new.objects - new.inuse;
	if (mode) {
		new.inuse = page->objects;
		new.freelist = NULL;
	} else {
		new.freelist = freelist;
	}

	/*
	 * new page:
	 * 32       31                  16                          0
	 * +--------+-------------------+---------------------------+
	 * |   1    |      objects      |          objects          |
	 * +--------+-------------------+---------------------------+
	 * |                        counters                        |
	 * +--------------------------------------------------------+
	 */
	new.frozen = 1;
	if (!__cmpxchg_double_slab(s, page,
			freelist, counters,
			new.freelist, new.counters,
			"acquire_slab"))
		return NULL;

	remove_partial(n, page);
	return freelist;
}

/*
 * Try to allocate a partial slab from a specific node.
 */
static void *get_partial_node(struct kmem_cache *s, struct kmem_cache_node *n,
				struct kmem_cache_cpu *c, gfp_t flags)
{
	struct page *page, *page2;
	void *object = NULL;
	unsigned int available = 0;
	int objects;

	if (!n || !n->nr_partial)
		return NULL;

	list_for_each_entry_safe(page, page2, &n->partial, lru) {
		void *t;

		t = acquire_slab(s, n, page, object == NULL, &objects);
		if (!t)
			break;

		available += objects;
		if (!object) {
			c->page = page;
			stat(s, ALLOC_FROM_PARTIAL);
			object = t;
		} else {
			printk("Need put_cpu_partial\n");
		}

		if (!kmem_cache_has_cpu_partial(s)
			|| available > slub_cpu_partial(s) / 2)
			break;
	}
	return object;
}

/*
 * Get a page from sonwhere. Search in increasing NUMA distances.
 */
static void *get_any_partial(struct kmem_cache *s, gfp_t flags,
			struct kmem_cache_cpu *c)
{
	return NULL;
}

/*
 * Get a partial page, lock it and return it.
 */
static void *get_partial(struct kmem_cache *s, gfp_t flags, int node,
				struct kmem_cache_cpu *c)
{
	void *object;

	object = get_partial_node(s, get_node(s, 0), c, flags);
	if (object)
		return object;

	return get_any_partial(s, flags, c);
}

static inline void *new_slab_objects(struct kmem_cache *s, gfp_t flags,
			int node, struct kmem_cache_cpu **pc)
{
	void *freelist;
	struct kmem_cache_cpu *c = *pc;
	struct page *page;

	freelist = get_partial(s, flags, node, c);
	if (freelist)
		return freelist;

	page = new_slab(s, flags, node);
	if (page) {
		c = s->cpu_slab;
		if (c->page)
			flush_slab(s, c);

		/*
		 * No other reference to the page yet so we can
		 * muck around with it freely without cmpxchg
		 */
		freelist = page->freelist;
		page->freelist = NULL;

		stat(s, ALLOC_SLAB);
		c->page = page;
		*pc = c;
	} else
		freelist = NULL;

	return freelist;
}

/*
 * Check the page->freelist of a page and either transfer the freelist to the
 * per cpu freelist or deactivate the page.
 *
 * The page is still frozen if the return value is not NULL.
 *
 * If this function returns NULL then the page has been unfrozen.
 *
 * This function must be called with interrupt disabled.
 */
static inline void *get_freelist(struct kmem_cache *s, struct page *page)
{
	struct page new;
	unsigned long counters;
	void *freelist;

	return page->freelist;
}

/*
 * Slow path. The lockless freelist is empty or we need to perform
 * debugging duties.
 *
 * Processing is still very fast if new objects have been freed to the
 * regular freelist. In that case we simply take over the regular freelist
 * as the lockless freelist and zap the regular freelist.
 *
 * If that is not working then we fail back to the partial lists. We take the
 * first element of the freelist as the object to allocate now and move the
 * rest of the freelist to the lockless freelist.
 *
 * And if we were unable to get a new slab from the partial slab lists then
 * we need to allocate a new slab. This is the slowest path since it involves
 * a call to the page allocator and the setup of a new slab.
 *
 * Version of __slab_alloc to use when we know that interrupts are
 * already disabled (which is the case for bulk allocation).
 */
static void *___slab_alloc(struct kmem_cache *s, gfp_t gfpflags, int node,
			unsigned long addr, struct kmem_cache_cpu *c)
{
	void *freelist;
	struct page *page;

	page = c->page;
	if (!page)
		goto new_slab;

redo:
	printk("Need %s\n", __func__);
	/* Must check again c->freelist in case of cpu migration or IRQ */
	freelist = c->freelist;
	if (freelist)
		goto load_freelist;

	freelist = get_freelist(s, page);

	if (!freelist) {
		c->page = NULL;
		stat(s, DEACTIVATE_BYPASS);
		goto new_slab;
	}
	stat(s, ALLOC_REFILL);

load_freelist:
	/*
	 * freelist is pointing to the list of objects to be used.
	 * page is pointing to the page from which the objects are obtained.
	 * That page must be frozen for per cpu allocations to work.
	 */
	c->freelist = get_freepointer(s, freelist);
	return freelist;

new_slab:

	if (slub_percpu_partial(c)) {
		page = c->page = slub_percpu_partial(c);
		slub_set_percpu_partial(c, page);
		stat(s, CPU_PARTIAL_ALLOC);
		goto redo;
	}

	freelist = new_slab_objects(s, gfpflags, node, &c);
	if (unlikely(!freelist)) {
		printk("SLUB out of memory.\n");
		return NULL;
	}

	page = c->page;
	goto load_freelist;
}

/*
 * Inlined fastpath so that allocation function (kmalloc, kmem_cache_alloc)
 * have the fastpath folded into their functions. So no function call
 * overhead for requests that can be satisfied on the fastpath.
 *
 * The fastpath works by first checking if the lockless freelist can be used.
 * If not then __slab_alloc is called for slow processing.
 *
 * Otherwise we can simply pick the next object from the lockless free list.
 */
static inline void *slab_alloc_node(struct kmem_cache *s,
		gfp_t gfpflags, int node, unsigned long addr)
{
	void *object;
	struct kmem_cache_cpu *c;
	struct page *page;
	unsigned long tid;

	if (!s)
		return NULL;
	c = s->cpu_slab;
	object = c->freelist;
	page = c->page;

	if (unlikely(!object)) {
		object = __slab_alloc(s, gfpflags, node, addr, c);
		stat(s, ALLOC_SLOWPATH);
	} else {
		void *next_obect = get_freepointer_safe(s, object);

		/*
		 * The cmpxchg will only match if there was no additional
		 * operation and if we are on the right processor.
		 *
		 * The cmpxchg does the following atomically (without lock
		 * semantics!)
		 */
	}

	if (unlikely(gfpflags & __GFP_ZERO) && object)
		memset(object, 0, s->object_size);

	return object;
}

static inline void *slab_alloc(struct kmem_cache *s,
			gfp_t gfpflags, unsigned long addr)
{
	return slab_alloc_node(s, gfpflags, -1, addr);
}

void *kmem_cache_alloc(struct kmem_cache *s, gfp_t gfpflags)
{
	void *ret = slab_alloc(s, gfpflags, _RET_IP_);

	return ret;
}

static int init_kmem_cache_nodes(struct kmem_cache *s)
{
	int node = 0;

	/* Emulate only 1 node */
	struct kmem_cache_node *n;

	if (slab_state == DOWN) {
		early_kmem_cache_node_alloc(node);
		return 1;
	}

	n = kmem_cache_alloc_node(kmem_cache_node, GFP_KERNEL, node);
	if (!n) {
		printk("Need %s\n", __func__);
		//free_kmem_cache_nodes(s);
		return 0;
	}

	init_kmem_cache_node(n);
	s->node[node] = n;
	return 1;
}

static inline int alloc_kmem_cache_cpus(struct kmem_cache *s)
{
	s->cpu_slab = malloc(ALIGN(sizeof(struct kmem_cache_cpu), 
					2 * sizeof(void *)));

	if (!s->cpu_slab)
		return 0;

	return 1;
}

static void free_kmem_cache_nodes(struct kmem_cache *s)
{
	printk("SDFASDFDDD\n");
}

static int kmem_cache_open(struct kmem_cache *s, slab_flags_t flags)
{
	s->flags = kmem_cache_flags(s->size, flags, s->name, s->ctor);

	if (!calculate_sizes(s, -1))
		goto error;

	/*
	 * The larger the object size is, the more pages we want on the partial
	 * list to avoid pounding the page allocator excessively.
	 */
	set_min_partial(s, ilog2(s->size) / 2);

	set_cpu_partial(s);

	/* Initialize the per-computed randomized freelist if slab is up */
	if (slab_state >= UP) {
		printk("SBBBB\n");
	}

	if (!init_kmem_cache_nodes(s))
		goto error;

	if (alloc_kmem_cache_cpus(s))
		return 0;
	
	free_kmem_cache_nodes(s);
	return 0;
error:
	return -1;
}

int __kmem_cache_create(struct kmem_cache *s, slab_flags_t flags)
{
	int err;

	err = kmem_cache_open(s, flags);
	if (err)
		return err;

	/* Mutex is not taken during early boot */
	if (slab_state <= UP)
		return 0;

	printk("Waiting running\n");

	return 0;
}

/* Create a cache during boot when no slab services are available yet */
void create_boot_cache(struct kmem_cache *s, const char *name,
		unsigned int size, slab_flags_t flags,
		unsigned int useroffset, unsigned int usersize)
{
	int err;

	s->name = name;
	s->size = s->object_size = size;
	s->align = calculate_alignment(flags, ARCH_KMALLOC_MINALIGN, size);
	s->useroffset = useroffset;
	s->usersize = usersize;

	err = __kmem_cache_create(s, flags);
	if (err)
		printk("Creation of kmalloc slab %s size=%u failed.\n",
							name, size);
	s->refcount = -1;	/* Exempt from merging for now */
}

static void remove_full(struct kmem_cache *s, struct kmem_cache_node *n,
					struct page *page)
{
}

static void discard_slab(struct kmem_cache *s, struct page *page)
{
	printk("Need %s\n", __func__);
}

/*
 * Tracking of fully allocated slabs for debugging purposes.
 */
static void add_full(struct kmem_cache *s,
		struct kmem_cache_node *n, struct page *page)
{
}

static inline void add_partial(struct kmem_cache_node *n,
					struct page *page, int tail)
{
	__add_partial(n, page, tail);
}

/*
 * Remove the cpu slab
 */
static void deactivate_slab(struct kmem_cache *s, struct page *page,
			void *freelist, struct kmem_cache_cpu *c)
{
	enum slab_modes { M_NONE, M_PARTIAL, M_FULL, M_FREE };
	struct kmem_cache_node *n = get_node(s, 0);
	enum slab_modes l = M_NONE, m = M_NONE;
	void *nextfree;
	int tail = DEACTIVATE_TO_HEAD;
	struct page new;
	struct page old;

	if (page->freelist) {
		stat(s, DEACTIVATE_REMOTE_FREES);
		tail = DEACTIVATE_TO_TAIL;
	}

	/*
	 * Stage one: Free all available per cpu objects back
	 * to the page freelist while it is still frozen. Leave the
	 * last one.
	 */
	while (freelist && (nextfree = get_freepointer(s, freelist))) {
		void *prior;
		unsigned long counters;

		do {
			prior = page->freelist;
			counters = page->counters;
			set_freepointer(s, freelist, prior);
			new.counters = counters;
			new.inuse--;
		} while (!__cmpxchg_double_slab(s, page,
				prior, counters, 
				freelist, new.counters,
				"drain percpu freelist"));
		freelist = nextfree;
	}

	/*
	 * Stage two: Ensure that the page is unfrozen while the
	 * list presence reflects the actual number of objects
	 * during unfreeze.
	 *
	 * We setup the list membership and then perform a cmpxchg
	 * with the count. If there is a mismatch then the page
	 * is not unfrozen but the page is on the wrong list.
	 *
	 * Then we restart the process which may have to remove
	 * the page from the list that we just put it on again
	 * because the number of objects in the slab may have
	 * changed.
	 */
redo:
	old.freelist = page->freelist;
	old.counters = page->counters;

	/* Determine target state of the slab */
	new.counters = old.counters;
	if (freelist) {
		new.inuse--;
		set_freepointer(s, freelist, old.freelist);
		new.freelist = freelist;
	} else
		new.freelist = old.freelist;

	new.frozen = 0;

	if (!new.inuse && n->nr_partial >= s->min_partial) {
		m = M_FREE;
	} else if (new.freelist) {
		m = M_PARTIAL;
	} else {
		m = M_FULL;
	}

	if (l != m) {
		if (l == M_PARTIAL)
			remove_partial(n, page);
		else if (l == M_FULL)
			remove_full(s, n, page);

		if (m == M_PARTIAL)
			add_partial(n, page, tail);
		else if (m == M_FULL)
			add_full(s, n, page);
	}

	l = m;
	if (!__cmpxchg_double_slab(s, page,
				old.freelist, old.counters,
				new.freelist, new.counters,
				"unfreezing slab"))
		goto redo;

	if (m == M_PARTIAL)
		stat(s, tail);
	else if (m == M_FULL)
		stat(s, DEACTIVATE_FULL);
	else if (m == M_FREE) {
		stat(s, DEACTIVATE_EMPTY);
		discard_slab(s, page);
		stat(s, FREE_SLAB);
	}

	c->page = NULL;
	c->freelist = NULL;
}

/*
 * Unfreeze all the cpu partial slabs.
 *
 * This function must be called with interrupts disabled
 * for the cpu using c (or some other guarantee must be there
 * to guarantee no concurrent accesses).
 */
static void unfreeze_partials(struct kmem_cache *s,
			struct kmem_cache_cpu *c)
{
	struct page *page;

	while ((page = c->partial))
		printk("NEED %s\n", __func__);
}

static inline void flush_slab(struct kmem_cache *s, struct kmem_cache_cpu *c)
{
	stat(s, CPUSLAB_FLUSH);
	deactivate_slab(s, c->page, c->freelist, c);

	unfreeze_partials(s, c);
}

/*
 * Flush cpu slab.
 *
 * Called from IPI handler with interrupts disabled.
 */
static inline void __flush_cpu_slab(struct kmem_cache *s, int cpu)
{
	struct kmem_cache_cpu *c = s->cpu_slab;

	if (c->page)
		flush_slab(s, c);
}

static struct kmem_cache *bootstrap(struct kmem_cache *static_cache)
{
	int node;
	struct kmem_cache *s = kmem_cache_zalloc(kmem_cache, GFP_NOWAIT);
	struct kmem_cache_node *n;

	memcpy(s, static_cache, kmem_cache->object_size);

	/*
	 * This runs very early, and only the boot processor is supposed to be
	 * up. Even if it weren't true.
	 */
	__flush_cpu_slab(s, 0);

	for_each_kmem_cache_node(s, node, n) {
		struct page *p;

		list_for_each_entry(p, &n->partial, lru)
			p->slab_cache = s;
	}
	list_add(&s->list, &slab_caches);
	return s;
}

static inline unsigned int size_index_elem(unsigned int bytes)
{
	return (bytes - 1) / 8;
}

/*
 * Conversion table for small slabs sizes / 8 to the index in the
 * kmalloc array. This is necessary for slabs < 192 since we have non power
 * of two cache sizes there. The size of larger slabs can be determined using
 * fls.
 */
static u8 size_index[24] = {
	3,	/* 8 */
	4,	/* 16 */
	5,	/* 24 */
	5,	/* 32 */
	6,	/* 40 */
	6,	/* 48 */
	6,	/* 56 */
	6,	/* 64 */
	1,	/* 72 */
	1,	/* 80 */
	1,	/* 88 */
	1,	/* 96 */
	7,	/* 104 */
	7,	/* 112 */
	7,	/* 120 */
	7,	/* 128 */
	2,	/* 136 */
	2,	/* 144 */
	2,	/* 152 */
	2,	/* 160 */
	2,	/* 168 */
	2,	/* 176 */
	2,	/* 184 */
	2	/* 192 */
};

/*
 * Patch up the size_index table if we have strange large alignment
 * requirements for the kmalloc array. This is only the case for
 * MIPS it seems. The standard arches will not generate any code here.
 *
 * Largest permitted alignment is 256 bytes due to the way we
 * handle the index determination for the smaller caches.
 *
 * Make sure the nothing crazy happens if someone starts tinkering
 * around with ARCH_KMALLOC_MINALIGN
 */
static void setup_kmalloc_cache_index_table(void)
{
	unsigned int i;

	if (KMALLOC_MIN_SIZE > 256 ||
		KMALLOC_MIN_SIZE & (KMALLOC_MIN_SIZE - 1))
		printk("BUILD_BUG_ON: %s\n", __func__);

	for (i = 8; i < KMALLOC_MIN_SIZE; i += 8) {
		unsigned int elem = size_index_elem(i);

		if (elem >= ARRAY_SIZE(size_index))
			break;
		size_index[elem] = KMALLOC_SHIFT_LOW;
	}

	if (KMALLOC_MIN_SIZE >= 64) {
		/*
		 * The 96 byte size cache is not used if the alignment
		 * is 64 byte.
		 */
		for (i = 64 + 8; i < 96; i+= 8)
			size_index[size_index_elem(i)] = 7;
	}

	if (KMALLOC_MIN_SIZE >= 128) {
		/*
		 * The 192 byte sized cache is not used for if the alignment
		 * is 128 byte. Redirect kmalloc to use the 256 byte cache
		 * instead.
		 */
		for (i = 128 + 8; i <= 192; i += 8)
			size_index[size_index_elem(i)] = 8;
	}
}

static struct kmem_cache *
kmalloc_caches[NR_KMALLOC_TYPES][KMALLOC_SHIFT_HIGH + 1];

/*
 * kmalloc_info[] is to make slub_debug=,kmalloc-xx option work at boot time.
 * kmalloc_index() supports up to 2^26=64MB, so the final entry of the table is
 * kmalloc-67108864.
 */
const struct kmalloc_info_struct kmalloc_info[] = {
	{NULL,                      0},
	{"kmalloc-96",             96},
	{"kmalloc-192",           192},
	{"kmalloc-8",               8},
	{"kmalloc-16",             16},
	{"kmalloc-32",             32},
	{"kmalloc-64",             64},
	{"kmalloc-128",           128},
	{"kmalloc-256",           256},
	{"kmalloc-512",           512},
	{"kmalloc-1k",           1024},
	{"kmalloc-2k",           2048},
	{"kmalloc-4k",           4096},
	{"kmalloc-8k",           8192},
	{"kmalloc-16k",         16384},
	{"kmalloc-32k",         32768},
	{"kmalloc-64k",         65536},
	{"kmalloc-128k",       131072},
	{"kmalloc-256k",       262144},
	{"kmalloc-512k",       524288},
	{"kmalloc-1M",        1048576},
	{"kmalloc-2M",        2097152},
	{"kmalloc-4M",        4194304},
	{"kmalloc-8M",        8388608},
	{"kmalloc-16M",      16777216},
	{"kmalloc-32M",      33554432},
	{"kmalloc-64M",      67108864}
};

static const char *
kmalloc_cache_name(const char *prefix, unsigned int size)
{
	static const char units[3] = "\0kM";
	int idx = 0;

	while (size >= 1024 && (size % 1024 == 0)) {
		size /= 1024;
		idx++;
	}

	return kasprintf(GFP_NOWAIT, "%s-%u%c", prefix, size, units[idx]);
}

struct kmem_cache *create_kmalloc_cache(const char *name,
		unsigned int size, slab_flags_t flags,
		unsigned int useroffset, unsigned int usersize)
{
	struct kmem_cache *s = kmem_cache_zalloc(kmem_cache, GFP_NOWAIT);
}

static void
new_kmalloc_cache(int idx, int type, slab_flags_t flags)
{
	const char *name;

	if (type == KMALLOC_RECLAIM) {
		flags |= SLAB_RECLAIM_ACCOUNT;
		name = kmalloc_cache_name("kmalloc-rcl",
						kmalloc_info[idx].size);
	} else {
		name = kmalloc_info[idx].name;
	}
	
	kmalloc_caches[type][idx] = create_kmalloc_cache(name,
					kmalloc_info[idx].size, flags, 0,
					kmalloc_info[idx].size);
}

/*
 * Create the kmalloc array. Some of the regular kmalloc arrays
 * may already have been created because they were need to
 * enable allocations for slab creation.
 */
void create_kmalloc_caches(slab_flags_t flags)
{
	int i, type;

	for (type = KMALLOC_NORMAL; type <= KMALLOC_RECLAIM; type++) {
		for (i = KMALLOC_SHIFT_LOW; i <= KMALLOC_SHIFT_HIGH; i++) {
			if (!kmalloc_caches[type][i])
				new_kmalloc_cache(i, type, flags);
		}
	}
}

/*
 * Find the kmem_cache structure that serves a given size of
 * allocation
 */
struct kmem_cache *kmalloc_slab(size_t size, gfp_t flags)
{
	unsigned int index;

	if (size <= 192) {
		if (!size)
			return ZERO_SIZE_PTR;
		index = size_index[size_index_elem(size)];
	} else {
		if (size > KMALLOC_MAX_CACHE_SIZE)
			return NULL;
		index = fls(size - 1);
	}

	return kmalloc_caches[kmalloc_type(flags)][index];
}

void *__kmalloc_track_caller(size_t size, gfp_t gfpflags, unsigned long caller)
{
	struct kmem_cache *s;
	void *ret;

	if (size > KMALLOC_MAX_CACHE_SIZE)
		printk("Need %s\n", __func__);

	s = kmalloc_slab(size, gfpflags);
	if (unlikely(ZERO_OR_NULL_PTR(s)))
		return s;

	printk("NEED %s\n", __func__);
}

void kmem_cache_init(void)
{
	static struct kmem_cache boot_kmem_cache;
	static struct kmem_cache boot_kmem_cache_node;

	kmem_cache_node = &boot_kmem_cache_node;
	kmem_cache = &boot_kmem_cache;

	create_boot_cache(kmem_cache_node, "kmem_cache_node",
		sizeof(struct kmem_cache_node), SLAB_HWCACHE_ALIGN, 0, 0);

	/* Able to allocate the per node structures */
	slab_state = PARTIAL;

	create_boot_cache(kmem_cache, "kmem_cache",
			offsetof(struct kmem_cache, node) +
			nr_node_ids * sizeof(struct kmem_cache_node *),
				SLAB_HWCACHE_ALIGN, 0, 0);

	kmem_cache = bootstrap(&boot_kmem_cache);
	kmem_cache_node = bootstrap(&boot_kmem_cache_node);

	/* Now we can use the kmem_cache to allocate kmalloc slabs */
	setup_kmalloc_cache_index_table();
	create_kmalloc_caches(0);
}


















