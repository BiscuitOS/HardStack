#ifndef _BISCUITOS_H
#define _BISCUITOS_H

#define INT_MAX		((int)(~0U>>1))

#define NULL	((void *)0)

#ifdef CONFIG_64BIT
#define BITS_PER_LONG   64
#else
#define BITS_PER_LONG   32
#endif

typedef unsigned int u32;
typedef unsigned short __u16;
typedef unsigned short u16;
typedef __u16 __le16;
typedef unsigned long long u64;
typedef int bool;

enum {
	false = 0,
	true  = 1
};

#define ZERO_SIZE_PTR	((void *)16)
#define ZERO_OR_NULL_PTR(x)	((unsigned long)(x) <=	\
					(unsigned long)ZERO_SIZE_PTR)

#define likely(x)		__builtin_expect(!!(x), 1)
#define unlikely(x)		__builtin_expect(!!(x), 0)
#define __packed		__attribute__((__packed__))
#define __aligned(x)		__attribute__((__aligned__(x)))
#define prefetch(x)		__builtin_prefetch(x)

#define cpu_to_le16(x)		((__le16)(__u16)(x))

#define do_div(n, base)					\
({							\
	unsigned int __base = (base);			\
	unsigned int __rem;				\
	__rem = ((unsigned long long)(n)) % __base;	\
	(n) = ((unsigned long long)(n)) / __base;	\
	__rem;						\
})

#define _U      0x01    /* upper */
#define _L      0x02    /* lower */
#define _D      0x04    /* digit */
#define _C      0x08    /* cntrl */
#define _P      0x10    /* punct */
#define _S      0x20    /* white space (space/lf/tab) */
#define _X      0x40    /* hex digit */
#define _SP     0x80    /* hard space (0x20) */

extern const unsigned char _ctype[];

#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

#define isalnum(c)	((__ismask(c)&(_U|_L|_D)) != 0)

static inline int isdigit(int c)
{
	return '0' <= c && c <= '9';
}

static inline int skip_atoi(const char **s)
{
	int i = 0;

	do {
		i = i*10 + *((*s)++) - '0';
	} while (isdigit(**s));

	return i;
}

static inline char _tolower(const char c)
{
	return c | 0x20;
}

#define MAX_ERRNO	4095
#define IS_ERR_VALUE(x)	unlikely((unsigned long)(void *)(x) >= (unsigned long)-MAX_ERRNO)

static inline void * ERR_PTR(long error)
{
	return (void *) error;
}

static inline long PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

static inline bool IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

#define is_kernel_rodata(x)	(0)
#define for_each_possible_cpu(i)	for(i = 0; i < 1; i++)

#define NUMA_NO_NODE		(-1)
#define _AT(T,X)		(X)

#endif
