/*
 * X86 KVM lapic
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/jump_label_ratelimit.h>

struct static_key_deferred apic_hw_disabled_bs __read_mostly;
struct static_key_deferred apic_sw_disabled_bs __read_mostly;

void kvm_lapic_init_bs(void)
{
	/* do not patch jump label more than once per second */
	jump_label_rate_limit(&apic_hw_disabled_bs, HZ);
	jump_label_rate_limit(&apic_sw_disabled_bs, HZ);
}
