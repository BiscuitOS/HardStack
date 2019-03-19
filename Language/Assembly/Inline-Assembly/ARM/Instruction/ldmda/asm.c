/*
 * Arm inline-assembly
 *
 * (C) 2019.03.15 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>

/*
 * LDM (Load Multiple) loads a non-empty subset, or possibly all, of the
 * general-purpose registers from sequential memory locations. It is 
 * useful for block loads, stack operations and procedure exit sequences.
 * The general-purpose registers loaded can include the PC. If they do, 
 * the word loaded for the PC is treated as an address and a branch occurs
 * to that address. In ARMv5 and above, bit[0] of the loaded value
 * determines whether execution continues after this branch in ARM state
 * or in Thumb state, as though a BX (loaded_value) instruction had been
 * executed (but see also The T and J bits on page A2-15 for operation on
 * non-T variants of ARMv5). In earlier versions of the architecture, 
 * bits[1:0] of the loaded value are ignored and execution continues in 
 * ARM state, as though the instruction MOV PC,(loaded_value) had been
 * executed.
 *
 * Syntax
 *   LDM{<cond>}<addressing_mode> <Rn>{!}, <registers>
 */

static unsigned long R0[10] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
				0x77, 0x88, 0x99, 0xaa};

static int debug_ldmda(void)
{
	unsigned long R1 = 0x0;
	unsigned long R2 = 0x0;
	unsigned long R3 = 0x0;
	int i;
	
	/* Emulate Stack */
	for (i = 0; i < 10; i++)
		printk("R0[%d] %#lx\n", i, R0[i]);
	/*
	 * LDMIA: Load memory into Register, and descrement After
	 *
	 *
	 *             +-------------+
	 *             |             |
	 *             +-------------+
	 *             |             |
	 *             +-------------+
	 *             |             |
	 *             +-------------+
	 *             |            -|------------> R3
	 *    R0[5]--> +-------------+
	 *             |            -|------------> R2
	 *             +-------------+
	 *             |            -|------------> R1
	 *             +-------------+
	 *             |             |
	 *             +-------------+
	 *             |             |
	 *             +-------------+
	 *             |             |
	 *    R0[0]--> +-------------+
	 *
	 * Pop stack into registe.
	 */
	__asm__ volatile ("ldmda %3!, {%0, %1, %2}"
			: "=r" (R1), "=r" (R2), "=r" (R3)
			: "r" (&R0[5]));

	printk("R1: %#lx R2: %#lx R3: %#lx\n", R1, R2, R3);

	return 0;
}
device_initcall(debug_ldmda);
