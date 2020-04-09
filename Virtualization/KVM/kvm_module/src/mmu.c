/*
 * X86 KVM MMU
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/export.h>
#include <asm/kvm_host.h>
#include "kvm/internal.h"
#include "kvm/mmu.h"

/*
 * When setting this variable to true it enables Two-Dimensional-Paging
 * where the hardware walks 2 page tables:
 * 1. the guest-virtual to guest-physical
 * 2. while doing 1. it walks guest-physical to host-physical
 * If the hardware supports that we don't need to do shadow paging.
 */
bool tdp_enabled_bs = false;

#define PT64_SECOND_AVAIL_BITS_SHIFT	52

/* The mask for the R/X bits in EPT PTEs */
#define PT64_EPT_READABLE_MASK		0x1ull
#define PT64_EPT_EXECUTABLE_MASK	0x4ull

/* make pte_list_desc fit well in cache line */
#define PTE_LIST_EXT	3

struct pte_list_desc {
	u64 *sptes[PTE_LIST_EXT];
	struct pte_list_desc *more;
};

static struct kmem_cache *pte_list_desc_cache_bs;
static struct kmem_cache *mmu_page_header_cache_bs;
static struct percpu_counter kvm_total_used_mmu_pages_bs;

static u64 __read_mostly shadow_nx_mask_bs;
static u64 __read_mostly shadow_x_mask_bs; /* mutual exclusive with nx_mask */
static u64 __read_mostly shadow_user_mask_bs;
static u64 __read_mostly shadow_accessed_mask_bs;
static u64 __read_mostly shadow_dirty_mask_bs;
static u64 __read_mostly shadow_mmio_mask_bs;
static u64 __read_mostly shadow_present_mask_bs;
static u64 __read_mostly shadow_mmio_value_bs;
static u64 __read_mostly shadow_me_mask_bs;

/*
 * SPTEs used by MMUs without A/D bits are marked with shadow_acc_track_value.
 * Non-present SPTEs with shadow_acc_track_value set are in place for access
 * tracking.
 */
static u64 __read_mostly shadow_acc_track_mask_bs;
static const u64 shadow_acc_track_value_bs = SPTE_SPECIAL_MASK;

/*
 * The mask/shift to use for saving the original R/X bits when marking the PTE
 * as not-present for access tracking purposes. We do not save the W bit as the
 * PTEs being access tracked also need to be dirty tracked, so the W bit will be
 * restored only when a write is attempted to the page.
 */
static const u64 shadow_acc_track_saved_bits_mask_bs = 
			PT64_EPT_READABLE_MASK | PT64_EPT_EXECUTABLE_MASK;
static const u64 shadow_acc_track_saved_bits_shift_bs = 
						PT64_SECOND_AVAIL_BITS_SHIFT;

/*
 * This mask must be set on all non-zero Non-Present or Reserved SPTEs in order
 * to guard against L1TF attacks.
 */
static u64 __read_mostly shadow_nonpresent_or_rsvd_mask_bs;

/*
 * The number of high-order 1 bits to use in the mask above.
 */
static const u64 shadow_nonpresent_or_rsvd_mask_len_bs = 5;

/*
 * In some cases, we need to preserve the GFN of a non-present or reserved
 * SPTE when we usurp the upper five bits of the physical address space to
 * defend against L1TF, e.g. for MMIO SPTEs.  To preserve the GFN, we'll
 * shift bits of the GFN that overlap with shadow_nonpresent_or_rsvd_mask
 * left into the reserved bits, i.e. the GFN in the SPTE will be split into
 * high and low parts.  This mask covers the lower bits of the GFN.
 */
static u64 __read_mostly shadow_nonpresent_or_rsvd_lower_gfn_mask_bs;

static void kvm_mmu_reset_all_pte_masks_bs(void)
{
	u8 low_phys_bits;

	shadow_user_mask_bs = 0;
	shadow_accessed_mask_bs = 0;
	shadow_dirty_mask_bs = 0;
	shadow_nx_mask_bs = 0;
	shadow_x_mask_bs = 0;
	shadow_mmio_mask_bs = 0;
	shadow_present_mask_bs = 0;
	shadow_present_mask_bs = 0;
	shadow_acc_track_mask_bs = 0;

	/*
	 * If the CPU has 46 or less physical address bits, then set an
	 * appropriate mask to guard against L1TF attacks. Otherwise, it is
	 * assumed that the CPU is not vulnerable to L1TF.
	 */
	low_phys_bits = boot_cpu_data.x86_phys_bits;
	if (boot_cpu_data.x86_phys_bits <
		52 - shadow_nonpresent_or_rsvd_mask_len_bs) {
		shadow_nonpresent_or_rsvd_mask_bs =
			rsvd_bits_bs(boot_cpu_data.x86_phys_bits -
				shadow_nonpresent_or_rsvd_mask_len_bs,
				boot_cpu_data.x86_phys_bits - 1);
		low_phys_bits -= shadow_nonpresent_or_rsvd_mask_len_bs;
	}
	shadow_nonpresent_or_rsvd_lower_gfn_mask_bs =
		GENMASK_ULL(low_phys_bits - 1, PAGE_SHIFT);
}

static unsigned long
mmu_shrink_count_bs(struct shrinker *shrink, struct shrink_control *sc)
{
	BS_DUP();
	return percpu_counter_read_positive(&kvm_total_used_mmu_pages_bs);
}

static unsigned long
mmu_shrink_scan_bs(struct shrinker *shrink, struct shrink_control *sc)
{
	BS_DUP();
	return 0;
}

static struct shrinker mmu_shrinker_bs = {
	.count_objects = mmu_shrink_count_bs,
	.scan_objects = mmu_shrink_scan_bs,
	.seeks = DEFAULT_SEEKS * 10,
};

static void mmu_destroy_caches_bs(void)
{
	kmem_cache_destroy(pte_list_desc_cache_bs);
	kmem_cache_destroy(mmu_page_header_cache_bs);
}

int kvm_mmu_module_init_bs(void)
{
	int ret = -ENOMEM;

	/*
	 * MMU roles use union aliasing which is, generally speaking, an
	 * undefined behavior. Hovever, we supposedly know how compilers
	 * behave and the current status quo is unlikely to change. Guardians
	 * below are supposed to let us know if the assumption becomes false.
	 */
	BUILD_BUG_ON(sizeof(union kvm_mmu_page_role) != sizeof(u32));
	BUILD_BUG_ON(sizeof(union kvm_mmu_extended_role) != sizeof(u32));
	BUILD_BUG_ON(sizeof(union kvm_mmu_role) != sizeof(u64));

	kvm_mmu_reset_all_pte_masks_bs();

	pte_list_desc_cache_bs = kmem_cache_create("pte_list_desc_bs",
					sizeof(struct pte_list_desc),
					0,
					SLAB_ACCOUNT,
					NULL);
	if (!pte_list_desc_cache_bs)
		goto out;

	mmu_page_header_cache_bs = kmem_cache_create("kvm_mmu_page_header_bs",
					sizeof(struct kvm_mmu_page),
					0,
					SLAB_ACCOUNT,
					NULL);
	if (!mmu_page_header_cache_bs)
		goto out;

	if (percpu_counter_init(&kvm_total_used_mmu_pages_bs, 0, GFP_KERNEL))
		goto out;

	ret = register_shrinker(&mmu_shrinker_bs);
	if (ret)
		goto out;

	return 0;

out:
	mmu_destroy_caches_bs();
	return ret;
}

void kvm_mmu_set_mmio_spte_mask_bs(u64 mmio_mask, u64 mmio_value)
{
	BUG_ON((mmio_mask & mmio_value) != mmio_value);
	shadow_mmio_value_bs = mmio_value | SPTE_SPECIAL_MASK;
	shadow_mmio_mask_bs = mmio_mask | SPTE_SPECIAL_MASK;
}
EXPORT_SYMBOL_GPL(kvm_mmu_set_mmio_spte_mask_bs);

/*
 * Sets the shadow PTE masks used by the MMU.
 *
 * Assumptions:
 *   - Setting either @accessed_mask or @dirty_mask reqiure setting both
 *   - At least one of @accessed_mask or @acc_track_mask must be set
 */
void kvm_mmu_set_mask_ptes_bs(u64 user_mask, u64 accessed_mask,
		u64 dirty_mask, u64 nx_mask, u64 x_mask, u64 p_mask,
		u64 acc_track_mask, u64 me_mask)
{
	BUG_ON(!dirty_mask != !accessed_mask);
	BUG_ON(!accessed_mask && !acc_track_mask);
	BUG_ON(acc_track_mask & shadow_acc_track_value_bs);

	shadow_user_mask_bs = user_mask;
	shadow_accessed_mask_bs = accessed_mask;
	shadow_dirty_mask_bs = accessed_mask;
	shadow_nx_mask_bs = nx_mask;
	shadow_x_mask_bs = x_mask;
	shadow_present_mask_bs = p_mask;
	shadow_acc_track_mask_bs = acc_track_mask;
	shadow_me_mask_bs = me_mask;
}
EXPORT_SYMBOL_GPL(kvm_mmu_set_mask_ptes_bs);

void kvm_enable_tdp_bs(void)
{
	tdp_enabled_bs = true;
}
EXPORT_SYMBOL_GPL(kvm_enable_tdp_bs);

void kvm_disable_tdp_bs(void)
{
	tdp_enabled_bs = false;
}
EXPORT_SYMBOL_GPL(kvm_disable_tdp_bs);
