#ifndef _BISCUITOS_HIGHMEM_H
#define _BISCUITOS_HIGHMEM_H

#define GOLDEN_RATIO_32	0x61C88647

static inline unsigned int __hash_32(unsigned int val)
{
	return val * GOLDEN_RATIO_32;
}

static inline unsigned int hash_32(unsigned int val, unsigned int bits)
{
	return __hash_32(val) >> (32 - bits);
}

#define hash_long(val, bits)	hash_32(val, bits)

static inline unsigned int hash_ptr(const void *ptr, unsigned int bits)
{
	return hash_long((unsigned long)ptr, bits);
}

static inline int PageHighMem(const struct page *page)
{
	if (page_zone(page) == &BiscuitOS_zone)
		return 0;
	else
		return 1;
}

extern void *page_address(const struct page *page);
#endif
