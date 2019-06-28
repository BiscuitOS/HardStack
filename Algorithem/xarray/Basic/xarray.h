/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef _XARRAY_H_
#define _XARRAY_H_

/* bitmap */
#include <bitmap.h>

/* Porting define not in XArray. */

enum {
	false	= 0,
	true	= 1
};
typedef _Bool		bool;
typedef unsigned	gfp_t;
typedef unsigned	xa_mark_t;

#undef NULL
#define NULL ((void *)0)

#define unlikely(x)	__builtin_expect(!!(x), 0)
#define likely(x)	__builtin_expect(!!(x), 1)

#define MAX_ERRNO	4095

#define WARN_ON(condition) ({						\
	int __ret_warn_on = !!(condition);				\
	unlikely(__ret_warn_on);					\
})

#define WARN_ON_ONCE(condition) WARN_ON(condition)

#define __GFP_BITS_SHIFT	(23)
#define __GFP_BITS_MASK		(((1 << __GFP_BITS_SHIFT) - 1))

#define ___GFP_IO		0x40u
#define ___GFP_FS		0x80u
#define ___GFP_DIRECT_RECLAIM	0x200000u
#define ___GFP_KSWAPD_RECLAIM	0x400000u
#define __GFP_DIRECT_RECLAIM	((gfp_t)___GFP_DIRECT_RECLAIM) /* Caller can reclaim */
#define __GFP_IO	((gfp_t)___GFP_IO)
#define __GFP_FS	((gfp_t)___GFP_FS)

#define __GFP_RECLAIM	((gfp_t)(___GFP_DIRECT_RECLAIM|___GFP_KSWAPD_RECLAIM))
#define GFP_KERNEL	(__GFP_RECLAIM | __GFP_IO | __GFP_FS)

static inline bool gfpflags_allow_blocking(const gfp_t gfp_flags)
{
	return !!(gfp_flags & __GFP_DIRECT_RECLAIM);
}

#define max(x, y)	((x) > (y) ? (x) : (y))
#define min(x, y)	((x) > (y) ? (y) : (x))

/*
 * xa_mk_internal() - Create an internal entry.
 * @v: Value to turn into an internal entry.
 *
 * Context: Any context.
 * Return: An XArray internal entry corresponding to this value.
 */
static inline void *xa_mk_internal(unsigned long v)
{
	return (void *)((v << 2) | 2);
}

static inline bool xa_is_internal(const void *entry);
/**
 * xa_is_err() - Report whether an XArray operation returned an error
 * @entry: Result from calling an XArray function
 *
 * If an XArray operation cannot complete an operation, it will return
 * a special value indicating an error.  This function tells you
 * whether an error occurred; xa_err() tells you which error occurred.
 *
 * Context: Any context.
 * Return: %true if the entry indicates an error.
 */
static inline bool xa_is_err(const void *entry)
{
	return unlikely(xa_is_internal(entry) &&
			entry >= xa_mk_internal(-MAX_ERRNO));
}

/**
 * xa_err() - Turn an XArray result into an errno.
 * @entry: Result from calling an XArray function.
 *
 * If an XArray operation cannot complete an operation, it will return
 * a special pointer value which encodes an errno.  This function extracts
 * the errno from the pointer value, or returns 0 if the pointer does not
 * represent an errno.
 *
 * Context: Any context.
 * Return: A negative errno or 0.
 */
static inline int xa_err(void *entry)
{
	/* xa_to_internal() would not do sign extension. */
	if (xa_is_err(entry))
		return (long)entry >> 2;
	return 0;
}

#define XA_MARK_0		(0U)
#define XA_MARK_1		(1U)
#define XA_MARK_2		(2U)
#define XA_PRESENT		(8U)
#define XA_MARK_MAX		XA_MARK_2
#define XA_FREE_MARK		XA_MARK_0

enum xa_lock_type {
	XA_LOCK_IRQ = 1,
	XA_LOCK_BH = 2,
};

/*
 * Values for xa_flags.  The radix tree stores its GFP flags in the xa_flags,
 * and we remain compatible with that.
 */

#define XA_FLAGS_LOCK_IRQ	(XA_LOCK_IRQ)
#define XA_FLAGS_LOCK_BH	(XA_LOCK_BH)
#define XA_FLAGS_TRACK_FREE	(4U)
#define XA_FLAGS_MARK(mark)	((1UL << __GFP_BITS_SHIFT) << \
						(unsigned)(mark))

/**
 * struct xarray - The anchor of the XArray.
 * @xa_lock: Lock that protects the contents of the XArray.
 *
 * To use the xarray, define it statically or embed it in your data structure.
 * It is a very small data structure, so it does not usually make sense to
 * allocate it separately and keep a pointer to it in your data structure.
 *
 * You may use the xa_lock to protect your own data structures as well.
 */
/*
 * If all of the entries in the array are NULL, @xa_head is a NULL pointer.
 * If the only non-NULL entry in the array is at index 0, @xa_head is that
 * entry.  If any other entry in the array is non-NULL, @xa_head points
 * to an @xa_node.
 */
struct xarray {
/* private: The rest of the data structure is not to be used directly. */
	gfp_t	xa_flags;
	void *	xa_head;
};

#define XARRAY_INIT(name, flags) {				\
	.xa_flags = flags,					\
	.xa_head = NULL,					\
}

/*
 * DEFINE_XARRAY_FLAGS() - Define an XArray with custom flags.
 * @name: A string that names your XArray.
 * @flags: XA_FLAG values.
 *
 * This is intended for file scope definitions of XArrays. It declares
 * and initialises an empty XArray with the chosen name and flags. It is
 * equivalent to calling xa_init_flags() on the array, but it does the 
 * initialisation at compiletime instead of runtime.
 */
#define DEFINE_XARRAY_FLAGS(name, flags)				\
	struct xarray name = XARRAY_INIT(name, flags)

/*
 * DEFINE_XARRAY() - Define an XArray.
 * @name: A string that names your XArray.
 *
 * This is intended for file scope definitions of XArrays. It declares
 * and initialises an empty XArray with the chosen name. It is equivalent
 * to calling xa_init() on the array, but it does the initialisation at
 * compiletime instead of runtime.
 */
#define DEFINE_XARRAY(name)	DEFINE_XARRAY_FLAGS(name, 0)

#define XA_DEBUG_ON(xa, x)		do { } while (0)
#define XA_NODE_BUG_ON(node, x)		do { } while (0)

#define XA_RETRY_ENTRY		xa_mk_internal(256)
#define XA_ZERO_ENTRY		xa_mk_internal(257)

/**
 * xa_is_zero() - Is the entry a zero entry?
 * @entry: Entry retrieved from the XArray
 *
 * Return: %true if the entry is a zero entry.
 */
static inline bool xa_is_zero(const void *entry)
{
	return unlikely(entry == XA_ZERO_ENTRY);
}

/**
 * xa_is_retry() - Is the entry a retry entry?
 * @entry: Entry retrieved from the XArray
 *
 * Return: %true if the entry is a retry entry.
 */
static inline bool xa_is_retry(const void *entry)
{
	return unlikely(entry == XA_RETRY_ENTRY);
}

/**
 * xa_is_advanced() - Is the entry only permitted for the advanced API?
 * @entry: Entry to be stored in the XArray.
 *
 * Return: %true if the entry cannot be stored by the normal API.
 */
static inline bool xa_is_advanced(const void *entry)
{
	return xa_is_internal(entry) && (entry <= XA_RETRY_ENTRY);
}

/*
 * The xarray is constructed out of a set of 'chunks' of pointers.  Choosing
 * the best chunk size requires some tradeoffs.  A power of two recommends
 * itself so that we can walk the tree based purely on shifts and masks.
 * Generally, the larger the better; as the number of slots per level of the
 * tree increases, the less tall the tree needs to be.  But that needs to be
 * balanced against the memory consumption of each node.  On a 64-bit system,
 * xa_node is currently 576 bytes, and we get 7 of them per 4kB page.  If we
 * doubled the number of slots per node, we'd get only 3 nodes per 4kB page.
 */
#define XA_CHUNK_SHIFT		(CONFIG_BASE_SMALL ? 4 : 6)
#define XA_CHUNK_SIZE		(1UL << XA_CHUNK_SHIFT)
#define XA_CHUNK_MASK		(XA_CHUNK_SIZE - 1)
#define XA_MAX_MARKS		3
#define XA_MARK_LONGS		DIV_ROUND_UP(XA_CHUNK_SIZE, BITS_PER_LONG)

/*
 * @count is the count of every non-NULL element in the ->slots array
 * whether that is a value entry, a retry entry, a user pointer,
 * a sibling entry or a pointer to the next level of the tree.
 * @nr_values is the count of every element in ->slots which is
 * either a value entry or a sibling of a value entry.
 */
struct xa_node {
	unsigned char	shift;		/* Bits remaining in each slot */
	unsigned char	offset;		/* Slot offset in parent */
	unsigned char	count;		/* Total entry count */
	unsigned char	nr_values;	/* Value entry count */
	struct xa_node	*parent;	/* NULL at top of tree */
	struct xarray	*array;		/* The arrary we belong to */
	void 		*slots[XA_CHUNK_SIZE];
	union {
		unsigned long	tags[XA_MAX_MARKS][XA_MARK_LONGS];
		unsigned long	marks[XA_MAX_MARKS][XA_MARK_LONGS];
	};
};

/**
 * typedef xa_update_node_t - A callback function from the XArray.
 * @node: The node which is being processed
 *
 * This function is called every time the XArray updates the count of
 * present and value entries in a node.  It allows advanced users to
 * maintain the private_list in the node.
 *
 * Context: The xa_lock is held and interrupts may be disabled.
 *          Implementations should not drop the xa_lock, nor re-enable
 *          interrupts.
 */
typedef void (*xa_update_node_t)(struct xa_node *node);

/*
 * The xa_state is opaque to its users.  It contains various different pieces
 * of state involved in the current operation on the XArray.  It should be
 * declared on the stack and passed between the various internal routines.
 * The various elements in it should not be accessed directly, but only
 * through the provided accessor functions.  The below documentation is for
 * the benefit of those working on the code, not for users of the XArray.
 *
 * @xa_node usually points to the xa_node containing the slot we're operating
 * on (and @xa_offset is the offset in the slots array).  If there is a
 * single entry in the array at index 0, there are no allocated xa_nodes to
 * point to, and so we store %NULL in @xa_node.  @xa_node is set to
 * the value %XAS_RESTART if the xa_state is not walked to the correct
 * position in the tree of nodes for this operation.  If an error occurs
 * during an operation, it is set to an %XAS_ERROR value.  If we run off the
 * end of the allocated nodes, it is set to %XAS_BOUNDS.
 */
struct xa_state {
	struct xarray *xa;
	unsigned long xa_index;
	unsigned long xa_shift;
	unsigned long xa_sibs;
	unsigned long xa_offset;
	unsigned long xa_pad;		/* Helps gcc generate better code */
	struct xa_node *xa_node;
	struct xa_node *xa_alloc;
	xa_update_node_t xa_update;
};

/*
 * We encode errnos in the xas->xa_node.  If an error has happened, we need to
 * drop the lock to fix it, and once we've done so the xa_state is invalid.
 */
#define XA_ERROR(errno)	((struct xa_node *)(((unsigned long)errno << 2) | 2UL))
#define XAS_BOUNDS	((struct xa_node *)1UL)
#define XAS_RESTART	((struct xa_node *)3UL)

#define __XA_STATE(array, index, shift, sibs)	{		\
	.xa = array,						\
	.xa_index = index,					\
	.xa_shift = shift,					\
	.xa_sibs = sibs,					\
	.xa_offset = 0,						\
	.xa_pad = 0,						\
	.xa_node = XAS_RESTART,					\
	.xa_alloc = NULL,					\
	.xa_update = NULL					\
}

/**
 * XA_STATE() - Declare an XArray operation state.
 * @name: Name of this operation state (usually xas).
 * @array: Array to operate on.
 * @index: Initial index of interest.
 *
 * Declare and initialise an xa_state on the stack.
 */
#define XA_STATE(name, array, index)			\
	struct xa_state name = __XA_STATE(array, index, 0, 0)

/* Everything below here is the Advanced API.  Proceed with caution. */
 
static inline bool xas_not_node(struct xa_node *node)
{
	return ((unsigned long)node & 3) || !node;
}

/* True if the node represents head-of-tree, RESTART or BOUNDS */
static inline bool xas_top(struct xa_node *node)
{
	return node <= XAS_RESTART;
}

/*
 * xa_is_internal() - Is the entry an internal entry?
 * @entry: XArray entry.
 *
 * Context: Any context.
 * Return: %true if the entry is an internal entry.
 */
static inline bool xa_is_internal(const void *entry)
{
	return ((unsigned long)entry & 3) == 2;
}

/**
 * xa_is_value() - Determine if an entry is a value.
 * @entry: XArray entry.
 *
 * Context: Any context.
 * Return: True if the entry is a value, false if it is a pointer.
 */
static inline bool xa_is_value(const void *entry)
{
	return (unsigned long)entry & 1;
}

/* Private */
static inline bool xa_is_node(const void *entry)
{
	return xa_is_internal(entry) && (unsigned long)entry > 4096;
}

/* Private */
static inline void *xa_mk_sibling(unsigned int offset)
{
	return xa_mk_internal(offset);
}

/* Private */
static inline struct xa_node *xa_to_node(const void *entry)
{
	return (struct xa_node *)((unsigned long)entry - 2);
}

/**
 * xa_marked() - Inquire whether any entry in this array has a mark set
 * @xa: Array
 * @mark: Mark value
 *
 * Context: Any context.
 * Return: %true if any entry has this mark set.
 */
static inline bool xa_marked(const struct xarray *xa, xa_mark_t mark)
{
	return xa->xa_flags & XA_FLAGS_MARK(mark);
}

/**
 * xas_error() - Return an errno stored in the xa_state.
 * @xas: XArray operation state.
 *
 * Return: 0 if no error has been noted.  A negative errno if one has.
 */
static inline int xas_error(const struct xa_state *xas)
{
	return xa_err(xas->xa_node);
}

/**     
 * xas_set_err() - Note an error in the xa_state.
 * @xas: XArray operation state.
 * @err: Negative error number.
 *              
 * Only call this function with a negative @err; zero or positive errors
 * will probably not behave the way you think they should.  If you want
 * to clear the error from an xa_state, use xas_reset().
 */
static inline void xas_set_err(struct xa_state *xas, long err)
{
	xas->xa_node = XA_ERROR(err);
}

/**
 * xa_is_sibling() - Is the entry a sibling entry?
 * @entry: Entry retrieved from the XArray
 *
 * Return: %true if the entry is a sibling entry.
 */
static inline bool xa_is_sibling(const void *entry)
{
	return 0;
}

/**
 * xas_invalid() - Is the xas in a retry or error state?
 * @xas: XArray operation state.
 *
 * Return: %true if the xas cannot be used for operations.
 */
static inline bool xas_invalid(const struct xa_state *xas)
{
	return (unsigned long)xas->xa_node & 3;
}

/**
 * xas_valid() - Is the xas a valid cursor into the array?
 * @xas: XArray operation state.
 *
 * Return: %true if the xas can be used for operations.
 */
static inline bool xas_valid(const struct xa_state *xas)
{
	return !xas_invalid(xas);
}

/* Private */
static inline void *xa_entry(const struct xarray *xa,
				const struct xa_node *node, unsigned int offset)
{
	XA_NODE_BUG_ON(node, offset >= XA_CHUNK_SIZE);
	return node->slots[offset];
}

/* Private */
static inline void *xa_entry_locked(const struct xarray *xa,
				const struct xa_node *node, unsigned int offset)
{
	XA_NODE_BUG_ON(node, offset >= XA_CHUNK_SIZE);
	return node->slots[offset];
}

/* Private */
static inline struct xa_node *xa_parent(const struct xarray *xa,
					const struct xa_node *node)
{
	return node->parent;
}

/* Private */
static inline struct xa_node *xa_parent_locked(const struct xarray *xa,
					const struct xa_node *node)
{
	return node->parent;
}

/*
 * xa_to_internal() - Extract the value from an internal entry.
 * @entry: XArray entry.
 *
 * Context: Any context.
 * Return: The value which was stored in the internal entry.
 */
static inline unsigned long xa_to_internal(const void *entry)
{
	return (unsigned long)entry >> 2;
}

/* Private */
static inline unsigned long xa_to_sibling(const void *entry)
{
	return xa_to_internal(entry);
}

/* Private */
static inline void *xa_mk_node(const struct xa_node *node)
{
	return (void *)((unsigned long)node | 2);
}

#define xa_lock_bh(xa)		do {} while (0)
#define xa_unlock_bh(xa)	do {} while (0)
#define xa_lock_irq(xa)		do {} while (0)
#define xa_unlock_irq(xa)	do {} while (0)
#define xa_lock(xa)		do {} while (0)
#define xa_unlock(xa)		do {} while (0)


#define xas_lock(xas)		xa_lock((xas)->xa)
#define xas_unlock(xas)		xa_unlock((xas)->xa)
#define xas_lock_bh(xas)	xa_lock_bh((xas)->xa)
#define xas_unlock_bh(xas)	xa_unlock_bh((xas)->xa)
#define xas_lock_irq(xas)	xa_lock_irq((xas)->xa)
#define xas_unlock_irq(xas)	xa_unlock_irq((xas)->xa) 

/* Private */
static inline void *xa_head(const struct xarray *xa)
{
	return xa->xa_head;
}

/* Private */
static inline void *xa_head_locked(const struct xarray *xa)
{
	return xa->xa_head;
}

/**
 * xas_reload() - Refetch an entry from the xarray.
 * @xas: XArray operation state.
 *
 * Use this function to check that a previously loaded entry still has
 * the same value.  This is useful for the lockless pagecache lookup where
 * we walk the array with only the RCU lock to protect us, lock the page,
 * then check that the page hasn't moved since we looked it up.
 *
 * The caller guarantees that @xas is still valid.  If it may be in an
 * error or restart state, call xas_load() instead.
 *
 * Return: The entry at this location in the xarray.
 */
static inline void *xas_reload(struct xa_state *xas)
{
	struct xa_node *node = xas->xa_node;

	if (node)
		return xa_entry(xas->xa, node, xas->xa_offset);
	return xa_head(xas->xa);
}

/* Private */
static inline unsigned int xas_find_chunk(struct xa_state *xas, bool advance,
		xa_mark_t mark)
{
	unsigned long *addr = xas->xa_node->marks[(unsigned)mark];
	unsigned int offset = xas->xa_offset;

	if (advance)
		offset++;
	if (XA_CHUNK_SIZE == BITS_PER_LONG) {
		if (offset < XA_CHUNK_SIZE) {
			unsigned long data = *addr & (~0UL << offset);

			if (data)
				return __ffs(data);
		}
		return XA_CHUNK_SIZE;
	}

	return find_next_bit(addr, XA_CHUNK_SIZE, offset);
}

/**
 * xas_reset() - Reset an XArray operation state.
 * @xas: XArray operation state.
 *
 * Resets the error or walk state of the @xas so future walks of the
 * array will start from the root.  Use this if you have dropped the
 * xarray lock and want to reuse the xa_state.
 *
 * Context: Any context.
 */
static inline void xas_reset(struct xa_state *xas)
{
	xas->xa_node = XAS_RESTART;
}

/**
 * xas_retry() - Retry the operation if appropriate.
 * @xas: XArray operation state. 
 * @entry: Entry from xarray.
 *
 * The advanced functions may sometimes return an internal entry, such as
 * a retry entry or a zero entry.  This function sets up the @xas to restart
 * the walk from the head of the array if needed.
 *
 * Context: Any context.
 * Return: true if the operation needs to be retried. 
 */
static inline bool xas_retry(struct xa_state *xas, const void *entry)
{
	if (xa_is_zero(entry))
		return true;
	if (!xa_is_retry(entry))
		return false;
	xas_reset(xas);
	return true;
}

extern void *xa_find(struct xarray *xa, unsigned long *indexp,
		unsigned long max, xa_mark_t filter);
extern void *xas_find(struct xa_state *xas, unsigned long max);
extern void *xa_store(struct xarray *xa, unsigned long index, 
		void *entry, gfp_t gfp);
extern void *xa_find_after(struct xarray *xa, unsigned long *indexp,
		unsigned long max, xa_mark_t filter);
extern void *xa_erase(struct xarray *xa, unsigned long index);

/*
 * xa_for_each_start() - Iterate over a portion of an XArray.
 * @xa: XArray.
 * @index: Index of @entry.
 * @entry: Entry retrieved from array.
 * @start: First index to retrieve from array.
 *
 * During the iteration, @entry will have the value of the entry stored
 * in @xa at @index.  You may modify @index during the iteration if you
 * want to skip or reprocess indices.  It is safe to modify the array
 * during the iteration.  At the end of the iteration, @entry will be set
 * to NULL and @index will have a value less than or equal to max.
 *
 * xa_for_each_start() is O(n.log(n)) while xas_for_each() is O(n).  You have
 * to handle your own locking with xas_for_each(), and if you have to unlock
 * after each iteration, it will also end up being O(n.log(n)).
 * xa_for_each_start() will spin if it hits a retry entry; if you intend to
 * see retry entries, you should use the xas_for_each() iterator instead.
 * The xas_for_each() iterator will expand into more inline code than
 * xa_for_each_start().
 *
 * Context: Any context.  Takes and releases the RCU lock.
 */
#define xa_for_each_start(xa, index, entry, start) 			\
	for (index = start,						\
	     entry = xa_find(xa, &index, ULONG_MAX, XA_PRESENT);	\
	     entry;							\
	     entry = xa_find_after(xa, &index, ULONG_MAX, XA_PRESENT))

/**
 * xa_for_each() - Iterate over present entries in an XArray.
 * @xa: XArray.
 * @index: Index of @entry.
 * @entry: Entry retrieved from array.
 *
 * During the iteration, @entry will have the value of the entry stored
 * in @xa at @index.  You may modify @index during the iteration if you want
 * to skip or reprocess indices.  It is safe to modify the array during the
 * iteration.  At the end of the iteration, @entry will be set to NULL and
 * @index will have a value less than or equal to max.
 *
 * xa_for_each() is O(n.log(n)) while xas_for_each() is O(n).  You have
 * to handle your own locking with xas_for_each(), and if you have to unlock
 * after each iteration, it will also end up being O(n.log(n)).  xa_for_each()
 * will spin if it hits a retry entry; if you intend to see retry entries,
 * you should use the xas_for_each() iterator instead.  The xas_for_each()
 * iterator will expand into more inline code than xa_for_each().
 *
 * Context: Any context.  Takes and releases the RCU lock.
 */
#define xa_for_each(xa, index, entry) \
	xa_for_each_start(xa, index, entry, 0)

#endif
