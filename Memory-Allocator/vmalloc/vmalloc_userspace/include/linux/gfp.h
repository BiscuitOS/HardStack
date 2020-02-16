#ifndef _BISCUITOS_GFP_H
#define _BISCUITOS_GFP_H

typedef unsigned long gfp_t;

#define ___GFP_DMA		0x01u
#define ___GFP_HIGHMEM		0x02u
#define ___GFP_DMA32		0x04u
#define ___GFP_RECLAIMABLE	0x10u
#define ___GFP_HIGH		0x20u
#define ___GFP_IO		0x40u
#define ___GFP_FS		0x80u
#define ___GFP_NOWARN		0x200u
#define ___GFP_RETRY_MAYFAIL	0x400u
#define ___GFP_NOFAIL		0x800u
#define ___GFP_NORETRY		0x1000u
#define ___GFP_MEMALLOC		0x2000u
#define ___GFP_COMP		0x4000u
#define ___GFP_ZERO		0x8000u
#define ___GFP_NOMEMALLOC	0x10000u
#define ___GFP_HARDWALL		0x20000u
#define ___GFP_THISNODE		0x40000u
#define ___GFP_ATOMIC		0x80000u
#define ___GFP_DIRECT_RECLAIM	0x200000u
#define ___GFP_KSWAPD_RECLAIM	0x400000u

#define __GFP_RECLAIM	((gfp_t)(___GFP_DIRECT_RECLAIM|___GFP_KSWAPD_RECLAIM))

#define __GFP_ZERO	((gfp_t)___GFP_ZERO)
#define __GFP_COMP	((gfp_t)___GFP_COMP)
#define __GFP_DMA	((gfp_t)___GFP_DMA)
#define __GFP_DMA32	((gfp_t)___GFP_DMA32)
#define __GFP_ATOMIC	((gfp_t)___GFP_ATOMIC)
#define __GFP_IO	((gfp_t)___GFP_IO)
#define __GFP_HIGH	((gfp_t)___GFP_HIGH)
#define __GFP_FS	((gfp_t)___GFP_FS)
#define __GFP_NOWARN	((gfp_t)___GFP_NOWARN)
#define __GFP_NOFAIL	((gfp_t)___GFP_NOFAIL)
#define __GFP_NORETRY	((gfp_t)___GFP_NORETRY)
#define __GFP_MEMALLOC	((gfp_t)___GFP_MEMALLOC)
#define __GFP_HARDWALL	((gfp_t)___GFP_HARDWALL)
#define __GFP_THISNODE	((gfp_t)___GFP_THISNODE)
#define __GFP_HIGHMEM	((gfp_t)___GFP_HIGHMEM)
#define __GFP_RETRY_MAYFAIL	((gfp_t)___GFP_RETRY_MAYFAIL)
#define __GFP_NOMEMALLOC	((gfp_t)___GFP_NOMEMALLOC)
#define __GFP_KSWAPD_RECLAIM	((gfp_t)___GFP_KSWAPD_RECLAIM)
#define __GFP_RECLAIMABLE	((gfp_t)___GFP_RECLAIMABLE)
#define __GFP_DIRECT_RECLAIM	((gfp_t)___GFP_DIRECT_RECLAIM)

#define __GFP_BITS_SHIFT	(23)
#define __GFP_BITS_MASK		((gfp_t)((1 << __GFP_BITS_SHIFT) - 1))

#define GFP_KERNEL	(__GFP_RECLAIM | __GFP_IO | __GFP_FS)
#define GFP_NOWAIT	(__GFP_KSWAPD_RECLAIM)
#define GFP_DMA		__GFP_DMA
#define GFP_DMA32	__GFP_DMA32

/*
 * The set of flags that only affect watermark checking and reclaim
 * behaviour. This is used by the MM to obey the caller constraints
 * about IO, FS and watermark checking while ignoring placement
 * hints such as HIGHMEM usage.
 */
#define GFP_RECLAIM_MASK (__GFP_RECLAIM|__GFP_HIGH|__GFP_IO|__GFP_FS|\
		__GFP_NOWARN|__GFP_RETRY_MAYFAIL|__GFP_NOFAIL|\
		__GFP_NORETRY|__GFP_MEMALLOC|__GFP_NOMEMALLOC|\
		__GFP_ATOMIC)

/* Control allocation cpuset and node placement constraints */
#define GFP_CONSTRAINT_MASK	(__GFP_HARDWALL|__GFP_THISNODE)

/* The GFP flags allowed during early boot */
#define GFP_BOOT_MASK	(__GFP_BITS_MASK & ~(__GFP_RECLAIM|__GFP_IO|__GFP_FS))

#endif
