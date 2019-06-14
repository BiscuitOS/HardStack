/*
 * lib/bitmap.c
 * Helper functions for bitmap.h.
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2.  See the file COPYING for more details.
 */
#include <bitmap.h>
#include <string.h>
#include <errno.h>

/**
 * DOC: bitmap introduction
 *
 * bitmaps provide an array of bits, implemented using an an
 * array of unsigned longs.  The number of valid bits in a
 * given bitmap does _not_ need to be an exact multiple of
 * BITS_PER_LONG.
 *
 * The possible unused bits in the last, partially used word
 * of a bitmap are 'don't care'.  The implementation makes
 * no particular effort to keep them zero.  It ensures that
 * their value will not affect the results of any operation.
 * The bitmap operations that return Boolean (bitmap_empty,
 * for example) or scalar (bitmap_weight, for example) results
 * carefully filter out these unused bits from impacting their
 * results.
 *
 * The byte ordering of bitmaps is more natural on little
 * endian architectures.  See the big-endian headers
 * include/asm-ppc64/bitops.h and include/asm-s390/bitops.h
 * for the best explanations of this ordering.
 */

int __bitmap_equal(const unsigned long *bitmap1,
		const unsigned long *bitmap2, unsigned int bits)
{
	unsigned int k, lim = bits / BITS_PER_LONG;

	for (k = 0; k < lim; ++k)
		if (bitmap1[k] != bitmap2[k])
			return 0;

	if (bits % BITS_PER_LONG)
		if ((bitmap1[k] ^ bitmap2[k]) & BITMAP_LAST_WORD_MASK(bits))
			return 0;

	return 1;
}

void __bitmap_complement(unsigned long *dst, const unsigned long *src, 
				unsigned int bits)
{
	unsigned int k, lim = BITS_TO_LONGS(bits);

	for (k = 0; k < lim; ++k)
		dst[k] = ~src[k];
}

/**
 * __bitmap_shift_right - logical right shift of the bits in a bitmap
 *   @dst : destination bitmap
 *   @src : source bitmap
 *   @shift : shift by this many bits
 *   @nbits : bitmap size, in bits
 *
 * Shifting right (dividing) means moving bits in the MS -> LS bit
 * direction.  Zeros are fed into the vacated MS positions and the
 * LS bits shifted off the bottom are lost.
 */
void __bitmap_shift_right(unsigned long *dst, const unsigned long *src,
			unsigned shift, unsigned nbits)
{
	unsigned k, lim = BITS_TO_LONGS(nbits);
	unsigned off = shift / BITS_PER_LONG, rem = shift % BITS_PER_LONG;
	unsigned long mask = BITMAP_LAST_WORD_MASK(nbits);

	for (k = 0; off + k < lim; ++k) {
		unsigned long upper, lower;

		/*
		 * If shift is not word aligned, take lower rem bits of
		 * word above and make them the top rem bits of result.
		 */
		if (!rem || off + k + 1 >= lim)
			upper = 0;
		else {
			upper = src[off + k + 1];
			if (off + k + 1 == lim - 1) {
				upper &= mask;
			}
			upper <<= (BITS_PER_LONG - rem);
		}
		lower = src[off + k];
		if (off + k == lim - 1)
			lower &= mask;
		lower >>= rem;
		dst[k] = lower | upper;
	}
	if (off)
		memset(&dst[lim - off], 0, off * sizeof(unsigned long));
}

/**
 * __bitmap_shift_left - logical left shift of the bits in a bitmap
 *   @dst : destination bitmap
 *   @src : source bitmap
 *   @shift : shift by this many bits
 *   @nbits : bitmap size, in bits
 *
 * Shifting left (multiplying) means moving bits in the LS -> MS
 * direction.  Zeros are fed into the vacated LS bit positions
 * and those MS bits shifted off the top are lost.
 */
void __bitmap_shift_left(unsigned long *dst, const unsigned long *src,
			unsigned int shift, unsigned int nbits)
{
	int k;
	unsigned int lim = BITS_TO_LONGS(nbits);
	unsigned int off = shift / BITS_PER_LONG, rem = shift % BITS_PER_LONG;

	for (k = lim - off - 1; k >= 0; --k) {
		unsigned long upper, lower;

		/*
		 * If shift is not word aligned, take upper rem bits of
		 * work below and make them the bottom rem bits of result.
		 */
		if (rem && k > 0)
			lower = src[k - 1] >> (BITS_PER_LONG - rem);
		else
			lower = 0;
		upper = src[k] << rem;
		dst[k + off] = lower | upper;
	}
	if (off)
		memset(dst, 0, off * sizeof(unsigned long));
}

int __bitmap_and(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits)
{
	unsigned int k;
	unsigned int lim = bits / BITS_PER_LONG;
	unsigned long result = 0;

	for (k = 0; k < lim; k++)
		result |= (dst[k] = bitmap1[k] & bitmap2[k]);
	if (bits % BITS_PER_LONG)
		result |= (dst[k] = bitmap1[k] & bitmap2[k] &
				BITMAP_LAST_WORD_MASK(bits));
	return result != 0;
}

int __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits)
{
	unsigned int k;
	unsigned int nr = BITS_TO_LONGS(bits);

	for (k = 0; k < nr; k++)
		dst[k] = bitmap1[k] | bitmap2[k];
}

void __bitmap_xor(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits)
{
	unsigned int k;
	unsigned int nr = BITS_TO_LONGS(bits);

	for (k = 0; k < nr; k++)
		dst[k] = bitmap1[k] ^ bitmap2[k];
}

int __bitmap_andnot(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits)
{
	unsigned int k;
	unsigned int lim = bits / BITS_PER_LONG;
	unsigned long result = 0;

	for (k = 0; k < lim; k++)
		result |= (dst[k] = bitmap1[k] & ~bitmap2[k]);
	if (bits % BITS_PER_LONG)
		result |= (dst[k] = bitmap1[k] & ~bitmap2[k] &
			   BITMAP_LAST_WORD_MASK(bits));
	return result != 0;
}

int __bitmap_intersects(const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int bits)
{
	unsigned int k, lim = bits / BITS_PER_LONG;

	for (k = 0; k < lim; ++k)
		if (bitmap1[k] & bitmap2[k])
			return 1;

	if (bits % BITS_PER_LONG)
		if ((bitmap1[k] & bitmap2[k]) & BITMAP_LAST_WORD_MASK(bits))
			return 1;

	return 0;
}

int __bitmap_subset(const unsigned long *bitmap1,
		    const unsigned long *bitmap2, unsigned int bits)
{
	unsigned int k, lim = bits / BITS_PER_LONG;

	for (k = 0; k < lim; ++k)
		if (bitmap1[k] & ~bitmap2[k])
			return 0;

	if (bits % BITS_PER_LONG)
		if ((bitmap1[k] & ~bitmap2[k]) & BITMAP_LAST_WORD_MASK(bits))
			return 0;

	return 1;
}

int __bitmap_weight(const unsigned long *bitmap, unsigned int bits)
{
	unsigned int k, lim = bits / BITS_PER_LONG;
	int w = 0;

	for (k = 0; k < lim; k++)
		w += hweight_long(bitmap[k]);

	if (bits % BITS_PER_LONG)
		w += hweight_long(bitmap[k] & BITMAP_LAST_WORD_MASK(bits));

	return w;
}

void __bitmap_set(unsigned long *map, unsigned int start, int len)
{
	unsigned long *p = map + BIT_WORD(start);
	const unsigned int size = start + len;
	int bits_to_set = BITS_PER_LONG - (start % BITS_PER_LONG);
	unsigned long mask_to_set = BITMAP_FIRST_WORD_MASK(start);

	while (len - bits_to_set >= 0) {
		*p |= mask_to_set;
		len -= bits_to_set;
		bits_to_set = BITS_PER_LONG;
		mask_to_set = ~0UL;
		p++;
	}
	if (len) {
		mask_to_set &= BITMAP_LAST_WORD_MASK(size);
		*p |= mask_to_set;
	}
}

void __bitmap_clear(unsigned long *map, unsigned int start, int len)
{
	unsigned long *p = map + BIT_WORD(start);
	const unsigned int size = start + len;
	int bits_to_clear = BITS_PER_LONG - (start % BITS_PER_LONG);
	unsigned long mask_to_clear = BITMAP_FIRST_WORD_MASK(start);

	while (len - bits_to_clear >= 0) {
		*p &= ~mask_to_clear;
		len -= bits_to_clear;
		bits_to_clear = BITS_PER_LONG;
		mask_to_clear = ~0UL;
		p++;
	}
	if (len) {
		mask_to_clear &= BITMAP_LAST_WORD_MASK(size);
		*p &= ~mask_to_clear;
	}
}

/*
 * This is a common helper function for find_next_bit, find_next_zero_bit, and
 * find_next_and_bit. The differences are:
 *  - The "invert" argument, which is XORed with each fetched word before
 *    searching it for one bits.
 *  - The optional "addr2", which is anded with "addr1" if present.
 */
static inline unsigned long _find_next_bit(const unsigned long *addr1, 
		const unsigned long *addr2, unsigned long nbits,
		unsigned long start, unsigned long invert)
{
	unsigned long tmp;

	if (unlikely(start >= nbits))
		return nbits;

	tmp = addr1[start / BITS_PER_LONG];
	if (addr2)
		tmp &= addr2[start / BITS_PER_LONG];
	tmp ^= invert;

	/* Handle 1st word */
	tmp &= BITMAP_FIRST_WORD_MASK(start);
	start = round_down(start, BITS_PER_LONG);

	while (!tmp) {
		start += BITS_PER_LONG;
		if (start >= nbits)
			return nbits;

		tmp = addr1[start / BITS_PER_LONG];
		if (addr2)
			tmp &= addr2[start / BITS_PER_LONG];
		tmp ^= invert;
	}

	return min(start + __ffs(tmp), nbits);
}

unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
				unsigned long offset)
{
	return _find_next_bit(addr, NULL, size, offset, ~0UL);
}

/*
 * Find the next set bit in a memory region.
 */
unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
				unsigned long offset)
{
	return _find_next_bit(addr, NULL, size, offset, 0UL);
}

/**
 * bitmap_find_next_zero_area_off - find a contiguous aligned zero area
 * @map: The address to base the search on
 * @size: The bitmap size in bits
 * @start: The bitnumber to start searching at
 * @nr: The number of zeroed bits we're looking for
 * @align_mask: Alignment mask for zero area
 * @align_offset: Alignment offset for zero area.
 *      
 * The @align_mask should be one less than a power of 2; the effect is that
 * the bit offset of all zero areas this function finds plus @align_offset
 * is multiple of that power of 2.
 */
unsigned long bitmap_find_next_zero_area_off(unsigned long *map,
		unsigned long size, unsigned long start, unsigned int nr,
		unsigned long align_mask, unsigned long align_offset)
{
	unsigned long index, end, i;
again:
	index = find_next_zero_bit(map, size, start);

	/* Align allocation */
	index = __ALIGN_MASK(index + align_offset, align_mask) - align_offset;

	end = index + nr;
	if (end > size)
		return end;
	i = find_next_bit(map, end, index);
	if (i < end) {
		start = i + 1;
		goto again;
	}
	return index;
}

/*
 * Bitmap printing & parsing functions: first version by Nadia Yvette Chambers,
 * second version by Paul Jackson, third by Joe Korty.
 */

#define CHUNKSZ				32
#define nbits_to_hold_value(val)	fls(val)
#define BASEDEC				10

const unsigned char _ctype[] = {
_C,_C,_C,_C,_C,_C,_C,_C,                                /* 0-7 */
_C,_C|_S,_C|_S,_C|_S,_C|_S,_C|_S,_C,_C,                 /* 8-15 */
_C,_C,_C,_C,_C,_C,_C,_C,                                /* 16-23 */
_C,_C,_C,_C,_C,_C,_C,_C,                                /* 24-31 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,                            /* 32-39 */
_P,_P,_P,_P,_P,_P,_P,_P,                                /* 40-47 */
_D,_D,_D,_D,_D,_D,_D,_D,                                /* 48-55 */
_D,_D,_P,_P,_P,_P,_P,_P,                                /* 56-63 */
_P,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U,              /* 64-71 */
_U,_U,_U,_U,_U,_U,_U,_U,                                /* 72-79 */
_U,_U,_U,_U,_U,_U,_U,_U,                                /* 80-87 */
_U,_U,_U,_P,_P,_P,_P,_P,                                /* 88-95 */
_P,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L,              /* 96-103 */
_L,_L,_L,_L,_L,_L,_L,_L,                                /* 104-111 */
_L,_L,_L,_L,_L,_L,_L,_L,                                /* 112-119 */
_L,_L,_L,_P,_P,_P,_P,_C,                                /* 120-127 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                        /* 128-143 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                        /* 144-159 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,    /* 160-175 */
_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,        /* 176-191 */
_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,        /* 192-207 */
_U,_U,_U,_U,_U,_U,_U,_P,_U,_U,_U,_U,_U,_U,_U,_L,        /* 208-223 */
_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,        /* 224-239 */
_L,_L,_L,_L,_L,_L,_L,_P,_L,_L,_L,_L,_L,_L,_L,_L};       /* 240-255 */

/**
 * hex_to_bin - convert a hex digit to its real value
 * @ch: ascii character represents hex digit
 *
 * hex_to_bin() converts one hex digit to its actual value or -1 in case of bad
 * input.
 */
int hex_to_bin(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	ch = tolower(ch);
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	return -1;
}

/**
 * __bitmap_parse - convert an ASCII hex string into a bitmap.
 * @buf: pointer to buffer containing string.
 * @buflen: buffer size in bytes.  If string is smaller than this
 *    then it must be terminated with a \0.
 * @is_user: location of buffer, 0 indicates kernel space
 * @maskp: pointer to bitmap array that will contain result.
 * @nmaskbits: size of bitmap, in bits.
 *
 * Commas group hex digits into chunks.  Each chunk defines exactly 32
 * bits of the resultant bitmask.  No chunk may specify a value larger
 * than 32 bits (%-EOVERFLOW), and if a chunk specifies a smaller value
 * then leading 0-bits are prepended.  %-EINVAL is returned for illegal
 * characters and for grouping errors such as "1,,5", ",44", "," and "".
 * Leading and trailing whitespace accepted, but not embedded whitespace.
 */
int __bitmap_parse(const char *buf, unsigned int buflen,
		int is_user, unsigned long *maskp,
		int nmaskbits)
{
	int c, old_c, totaldigits, ndigits, nchunks, nbits;
	u32 chunk;
	const char *ubuf = (const char *)buf;

	bitmap_zero(maskp, nmaskbits);

	nchunks = nbits = totaldigits = c = 0;
	do {
		chunk = 0;
		ndigits = totaldigits;

		/* Get the next chunk of the bitmap */
		while (buflen) {
			old_c = c;
			c = *buf++;
			buflen--;
			if (isspace(c))
				continue;
			/*
			 * If the last character was a pace and the current
			 * character isn't '\0', we've got embedded whitespace.
			 * This is a no-no, so throw an error.
			 */
			if (totaldigits && c && isspace(old_c))
				return -EINVAL;

			/* A '\0' or a ',' signal the ned of the chunk */
			if (c == '\0' || c == ',')
				break;

			if (!isxdigit(c))
				return -EINVAL;

			/*
			 * Make sure there are at least 4 free bits in 'chunk'
			 * If not, this hexdigit will overflow 'chunk', so
			 * throw an error.
			 */
			if (chunk & ~((1UL << (CHUNKSZ - 4)) - 1))
				return -EOVERFLOW;

			chunk = (chunk << 4) | hex_to_bin(c);
			totaldigits++;
		}
		if (ndigits == totaldigits)
			return -EINVAL;
		if (nchunks == 0 && chunk == 0)
			continue;

		__bitmap_shift_left(maskp, maskp, CHUNKSZ, nmaskbits);
		*maskp |= chunk;
		nchunks++;
		nbits += (nchunks == 1) ? nbits_to_hold_value(chunk) : CHUNKSZ;
		if (nbits > nmaskbits)
			return -EOVERFLOW;
	} while (buflen && c == ',');

	return 0;
}

/**
 * bitmap_pos_to_ord - find ordinal of set bit at given position in bitmap
 *      @buf: pointer to a bitmap
 *      @pos: a bit position in @buf (0 <= @pos < @nbits)
 *      @nbits: number of valid bit positions in @buf
 *
 * Map the bit at position @pos in @buf (of length @nbits) to the
 * ordinal of which set bit it is.  If it is not set or if @pos
 * is not a valid bit position, map to -1.
 *
 * If for example, just bits 4 through 7 are set in @buf, then @pos
 * values 4 through 7 will get mapped to 0 through 3, respectively,
 * and other @pos values will get mapped to -1.  When @pos value 7
 * gets mapped to (returns) @ord value 3 in this example, that means
 * that bit 7 is the 3rd (starting with 0th) set bit in @buf.
 *
 * The bit positions 0 through @bits are valid positions in @buf.
 */
static int bitmap_pos_to_ord(const unsigned long *buf, unsigned int pos,
					unsigned int nbits)
{
	if (pos >= nbits || !test_bit(pos, buf))
		return -1;

	return __bitmap_weight(buf, pos);
}

/*      
 * Find the first set bit in a memory region.
 */
unsigned long find_first_bit(const unsigned long *addr, unsigned long size)
{
	unsigned long idx;

	for (idx = 0; idx * BITS_PER_LONG < size; idx++) {
		if (addr[idx])
			return min(idx * BITS_PER_LONG + __ffs(addr[idx]), 
									size);
	}
	return size;
}































