/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BISCUITOS_MMU_H
#define _BISCUITOS_MMU_H

#define PT64_PT_BITS		9
#define PT64_ENT_PER_PAGE	(1 << PT64_PT_BITS)
#define PT32_PT_BITS		10
#define PT32_ENT_PER_PAGE	(1 << PT32_PT_BITS)

#define PT_WRITABLE_SHIFT	1
#define PT_USER_SHIFT		2

#define PT_PRESENT_MASK		(1ULL << 0)
#define PT_WRITABLE_MASK	(1ULL << PT_WRITABLE_SHIFT)
#define PT_USER_MASK		(1ULL << PT_USER_SHIFT)
#define PT_PWT_MASK		(1ULL << 3)
#define PT_PCD_MASK		(1ULL << 4)
#define PT_ACCESSED_SHIFT	5
#define PT_ACCESSED_MASK	(1ULL << PT_ACCESSED_SHIFT)
#define PT_DIRTY_SHIFT		6
#define PT_DIRTY_MASK		(1ULL << PT_DIRTY_SHIFT)
#define PT_PAGE_SIZE_SHIFT	7
#define PT_PAGE_SIZE_MASK	(1ULL << PT_PAGE_SIZE_SHIFT)
#define PT_PAT_MASK		(1ULL << 7)
#define PT_GLOBAL_MASK		(1ULL << 8)
#define PT64_NX_SHIFT		63
#define PT64_NX_MASK		(1ULL << PT64_NX_SHIFT)

#define PT_PAT_SHIFT		7
#define PT_DIR_PAT_SHIFT	12
#define PT_DIR_PAT_MASK		(1ULL << PT_DIR_PAT_SHIFT)

#define PT32_DIR_PSE36_SIZE	4
#define PT32_DIR_PSE36_SHIFT	13
#define PT32_DIR_PSE36_MASK					\
	(((1ULL << PT32_DIR_PSE36_SIZE) - 1) << PT32_DIR_PSE36_SHIFT)

#define PT64_ROOT_5LEVEL	5
#define PT64_ROOT_4LEVEL	4
#define PT32_ROOT_LEVEL		2
#define PT32E_ROOT_LEVEL	3

static inline u64 rsvd_bits_bs(int s, int e)
{
	if (e < s)
		return 0;
	return ((1ULL << (e - s + 1)) - 1) << s;
}

#endif
