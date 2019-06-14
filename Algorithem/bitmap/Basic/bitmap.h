/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BITMAP_H_
#define _BITMAP_H_

/* Helper for export */

#ifdef CONFIG_64BIT
#define BITS_PER_LONG	64

/* Determine aarch/x64 */
#if (defined __arm__) | (defined __i386__)
#undef BITS_PER_LONG
#define BITS_PER_LONG	32
#endif /* Cross-Compiler not 64 bit */

#else /* 32 bits */
#define BITS_PER_LONG	32

/* Determine arm/i386 */
#if (defined __aarch64__) | (defined __x64_64__)
#undef BITS_PER_LONG
#define BITS_PER_LONG	64
#endif

#endif /* BITS_PER_LONG */

#define __KERNEL_DIV_ROUND_UP(n, d)	(((n) + (d) - 1) / (d))
#define DIV_ROUND_UP	__KERNEL_DIV_ROUND_UP

/*
 * Compile time versions of __arch_hweightN()
 */
#define __const_hweight8(w)		\
	((unsigned int)			\
	((!!((w) & (1ULL << 0))) +	\
	(!!((w) & (1ULL << 1))) +	\
	(!!((w) & (1ULL << 2))) +	\
	(!!((w) & (1ULL << 3))) +	\
	(!!((w) & (1ULL << 4))) +	\
	(!!((w) & (1ULL << 5))) +	\
	(!!((w) & (1ULL << 6))) +	\
	(!!((w) & (1ULL << 7)))))

#define __const_hweight16(w) (__const_hweight8(w)  + __const_hweight8((w)  >> 8 ))
#define __const_hweight32(w) (__const_hweight16(w) + __const_hweight16((w) >> 16))
#define __const_hweight64(w) (__const_hweight32(w) + __const_hweight32((w) >> 32)) 

#define hweight32(w) __const_hweight32(w)
#define hweight64(w) __const_hweight64(w)

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

/**
 * __ffs - find first bit in word.
 * @word: The word to search
 *              
 * Undefined if no bit exists, so code should check against 0 first.
 */  
static inline unsigned long __ffs(unsigned long word)
{
	return __builtin_ctzl(word);
}

/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */ 
static inline int fls(unsigned int x)
{
	return x ? sizeof(x) * 8 - __builtin_clz(x) : 0;
}

/**
 * __fls - find last (most-significant) set bit in a long word
 * @word: the word to search
 *
 * Undefined if no set bit exists, so code should check against 0 first.
 */
static inline unsigned long __fls(unsigned long word)
{
	return (sizeof(word) * 8) - 1 - __builtin_clzl(word);
}

#define min(x,y)	((x) > (y) ? (y) : (x))
#define max(x,y)	((x) > (y) ? (x) : (y))

/*
 * This looks more complex than it should be. But we need to
 * get the type for the ~ right in round_down (it needs to be
 * as wide as the result!), and we want to evaluate the macro
 * arguments just once each.
 */
#define __round_mask(x, y)	((__typeof(x))((y) - 1))

/**                     
 * round_down - round down to next specified power of 2
 * @x: the value to round
 * @y: multiple to round down to (must be a power of 2)
 *      
 * Rounds @x down to next multiple of @y (which must be a power of 2).
 * To perform arbitrary rounding down, use rounddown() below.
 */
#define round_down(x, y)	((x) & ~__round_mask(x, y))

#define __ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define __ALIGN_MASK(x, mask)	__ALIGN_KERNEL_MASK((x), (mask))

#define _U	0x01	/* upper */
#define _L	0x02	/* lower */
#define _D	0x04	/* digit */
#define _C	0x08	/* cntrl */
#define _P	0x10	/* punct */
#define _S	0x20	/* white space (space/lf/tab) */
#define _X	0x40	/* hex digit */
#define _SP	0x80	/* hard space (0x20) */

extern const unsigned char _ctype[];

#define __ismask(x)	(_ctype[(int)(unsigned char)(x)])
#define isspace(c)	((__ismask(c)&(_S)) != 0)
#define isxdigit(c)	((__ismask(c)&(_D|_X)) != 0)
#define isupper(c)	((__ismask(c)&(_U)) != 0)

static inline unsigned char __tolower(unsigned char c)
{
	if (isupper(c))
		c -= 'A' - 'a';
	return c;
}

#define tolower(c)	__tolower(c)

typedef unsigned int u32;

/*
 * bitmaps provide bit arrays that consume one or more unsigned
 * longs.  The bitmap interface and available operations are listed
 * here, in bitmap.h
 *
 * Function implementations generic to all architectures are in
 * lib/bitmap.c.  Functions implementations that are architecture
 * specific are in various include/asm-<arch>/bitops.h headers
 * and other arch/<arch> specific files.
 *
 * See lib/bitmap.c for more details.
 */

/**
 * DOC: bitmap overview
 *
 * The available bitmap operations and their rough meaning in the
 * case that the bitmap is a single unsigned long are thus:
 *
 * The generated code is more efficient when nbits is known at
 * compile-time and at most BITS_PER_LONG.
 *
 * ::
 *
 *  bitmap_zero(dst, nbits)                     *dst = 0UL
 *  bitmap_fill(dst, nbits)                     *dst = ~0UL
 *  bitmap_copy(dst, src, nbits)                *dst = *src
 *  bitmap_and(dst, src1, src2, nbits)          *dst = *src1 & *src2
 *  bitmap_or(dst, src1, src2, nbits)           *dst = *src1 | *src2
 *  bitmap_xor(dst, src1, src2, nbits)          *dst = *src1 ^ *src2
 *  bitmap_andnot(dst, src1, src2, nbits)       *dst = *src1 & ~(*src2)
 *  bitmap_complement(dst, src, nbits)          *dst = ~(*src)
 *  bitmap_equal(src1, src2, nbits)             Are *src1 and *src2 equal?
 *  bitmap_intersects(src1, src2, nbits)        Do *src1 and *src2 overlap?
 *  bitmap_subset(src1, src2, nbits)            Is *src1 a subset of *src2?
 *  bitmap_empty(src, nbits)                    Are all bits zero in *src?
 *  bitmap_full(src, nbits)                     Are all bits set in *src?
 *  bitmap_weight(src, nbits)                   Hamming Weight: number set bits
 *  bitmap_set(dst, pos, nbits)                 Set specified bit area
 *  bitmap_clear(dst, pos, nbits)               Clear specified bit area
 *  bitmap_find_next_zero_area(buf, len, pos, n, mask)  Find bit free area
 *  bitmap_find_next_zero_area_off(buf, len, pos, n, mask)  as above
 *  bitmap_shift_right(dst, src, n, nbits)      *dst = *src >> n
 *  bitmap_shift_left(dst, src, n, nbits)       *dst = *src << n
 *  bitmap_remap(dst, src, old, new, nbits)     *dst = map(old, new)(src)
 *  bitmap_bitremap(oldbit, old, new, nbits)    newbit = map(old, new)(oldbit)
 *  bitmap_onto(dst, orig, relmap, nbits)       *dst = orig relative to relmap
 *  bitmap_fold(dst, orig, sz, nbits)           dst bits = orig bits mod sz
 *  bitmap_parse(buf, buflen, dst, nbits)       Parse bitmap dst from kernel buf
 *  bitmap_parse_user(ubuf, ulen, dst, nbits)   Parse bitmap dst from user buf
 *  bitmap_parselist(buf, dst, nbits)           Parse bitmap dst from kernel buf
 *  bitmap_parselist_user(buf, dst, nbits)      Parse bitmap dst from user buf
 *  bitmap_find_free_region(bitmap, bits, order)  Find and allocate bit region
 *  bitmap_release_region(bitmap, pos, order)   Free specified bit region
 *  bitmap_allocate_region(bitmap, pos, order)  Allocate specified bit region
 *  bitmap_from_arr32(dst, buf, nbits)          Copy nbits from u32[] buf to dst
 *  bitmap_to_arr32(buf, src, nbits)            Copy nbits from buf to u32[] dst
 *
 * Note, bitmap_zero() and bitmap_fill() operate over the region of
 * unsigned longs, that is, bits behind bitmap till the unsigned long
 * boundary will be zeroed or filled as well. Consider to use
 * bitmap_clear() or bitmap_set() to make explicit zeroing or filling
 * respectively.
 */

/**
 * DOC: bitmap bitops
 *
 * Also the following operations in asm/bitops.h apply to bitmaps.::
 *
 *  set_bit(bit, addr)                  *addr |= bit
 *  clear_bit(bit, addr)                *addr &= ~bit
 *  change_bit(bit, addr)               *addr ^= bit
 *  test_bit(bit, addr)                 Is bit set in *addr?
 *  test_and_set_bit(bit, addr)         Set bit and return old value
 *  test_and_clear_bit(bit, addr)       Clear bit and return old value
 *  test_and_change_bit(bit, addr)      Change bit and return old value
 *  find_first_zero_bit(addr, nbits)    Position first zero bit in *addr
 *  find_first_bit(addr, nbits)         Position first set bit in *addr
 *  find_next_zero_bit(addr, nbits, bit)
 *                                      Position next zero bit in *addr >= bit
 *  find_next_bit(addr, nbits, bit)     Position next set bit in *addr >= bit
 *  find_next_and_bit(addr1, addr2, nbits, bit)
 *                                      Same as find_next_bit, but in
 *                                      (*addr1 & *addr2)
 *
 */

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))
#define BITMAP_LAST_WORD_MASK(nbits)  (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))

#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE		8
#define BITS_PER_TYPE(type)	(sizeof(type) * BITS_PER_BYTE)
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_TYPE(long))

static inline unsigned long hweight_long(unsigned long w)
{
	return sizeof(w) == 4 ? hweight32(w) : hweight64(w);
}

/* Import memset */
#include <string.h>

static inline void bitmap_zero(unsigned long *dst, unsigned int nbits)
{
	unsigned int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
	memset(dst, 0, len);
}

/**
 * test_bit - Determine whether a bit is set
 * @nr: bit number to test
 * @addr: Address to start counting from
 */
static inline int test_bit(int nr, const volatile unsigned long *addr)
{
	return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG - 1)));
}

extern int __bitmap_equal(const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits);
extern void __bitmap_complement(unsigned long *dst, const unsigned long *src,
			unsigned int bits);
extern void __bitmap_shift_right(unsigned long *dst, const unsigned long *src,
			unsigned shift, unsigned nbits);
extern void __bitmap_shift_left(unsigned long *dst, const unsigned long *src,
			unsigned int shift, unsigned int nbits);
extern int __bitmap_and(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits);
extern int __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits);
extern void __bitmap_xor(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits);
extern int __bitmap_andnot(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits);
extern int __bitmap_intersects(const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits);
extern int __bitmap_subset(const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits);
extern int __bitmap_weight(const unsigned long *bitmap, unsigned int bits);
extern void __bitmap_set(unsigned long *map, unsigned int start, int len);
extern void __bitmap_clear(unsigned long *map, unsigned int start, int len);
extern unsigned long find_next_zero_bit(const unsigned long *addr,
			unsigned long size, unsigned long offset);
extern unsigned long find_next_bit(const unsigned long *addr, 
			unsigned long size, unsigned long offset);
extern unsigned long bitmap_find_next_zero_area_off(unsigned long *map,
		unsigned long size, unsigned long start, unsigned int nr,
		unsigned long align_mask, unsigned long align_offset);
extern int __bitmap_parse(const char *buf, unsigned int buflen,
		int is_user, unsigned long *maskp, int nmaskbits);
extern unsigned long find_first_bit(const unsigned long *addr, 
		unsigned long size);
#endif
