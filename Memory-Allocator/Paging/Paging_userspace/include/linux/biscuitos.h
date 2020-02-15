#ifndef _BISCUITOS_H
#define _BISCUITOS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long long u64;
typedef unsigned int u32;
typedef int bool;

enum {
	false,
	true
};

#ifdef CONFIG_PHYS_ADDR_T_64BIT
typedef u64 phys_addr_t;
#else
typedef u32 phys_addr_t;
#endif

#define ULLONG_MAX	(~0ULL)
#define PHYS_ADDR_MAX	(~(phys_addr_t)0)

/* Emulate printk from printf */
#define printk(...)	printf(__VA_ARGS__)

/* Error */
#define ENOMEM		12	/* Out of memory */
/* BUG_ON/WARN */
#define WARN_ON(condition)	({		\
	int __ret_warn_on = !!(condition);	\
	unlikely(__ret_warn_on);		\
})

#define BUG() do {} while (1)
#define BUG_ON(condition)	do { if (condition) BUG(); } while (0)

#define ALIGN(x, a)		(((x) + ((a) - 1)) & ~((a) - 1))
#define IS_ALIGNED(x, a)	(((x) & ((typeof(x))(a) - 1)) == 0)

#define NODES_SHIFT		0
#define MAX_NUMNODES		(1 << NODES_SHIFT)
#define NUMA_NO_NODE		(-1)

#define max(x, y)	({x > y ? x : y; })
#define min(x, y)	({x < y ? x : y; })
#define max_t(type, x, y)	({			\
		type __max1 = (x);			\
		type __max2 = (y);			\
		__max1 > __max2 ? __max1 : __max2;	\
})
#define min_t(type, x, y)	({			\
		type __min1 = (x);			\
		type __min2 = (y);			\
		__min1 < __min2 ? __min1 : __min2;	\
})

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#define __aligned(x)	__attribute__((__aligned__(x)))

#define clamp(val, lo, hi)	min((typeof(val))max(val, lo), hi)
#define __round_mask(x, y)	((__typeof__(x))((y)-1))
#define round_up(x, y)		((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y)	((x) & ~__round_mask(x, y))

#define NR_CPUS		CONFIG_NR_CPUS
#define _AT(T,X)	(X)
#define _AC(X,Y)	(X)

#define DOMAIN_KERNEL	2
#define DOMAIN_USER	1
#define DOMAIN_IO	0
#define DOMAIN_VECTORS	3

#endif
