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

#include <linux/mm.h>

/*
 * LDR (Load Register) loads a word from a memory address. If the PC is 
 * specified as register <Rd> , the instruction loads a data word which 
 * it treats as an address, then branches to that address. In ARMv5T and
 * above, bit[0] of the loaded value determines whether execution continues
 * after this branch in ARM state or in Thumb state, as though a BX (
 * loaded_value) instruction had been executed. In earlier versions of
 * the architecture, bits[1:0] of the loaded value are ignored and 
 * execution continues in ARM state, as though a MOV PC,(loaded_value)
 * instruction had been executed.
 *
 * Syntax
 *   LDR{<cond>} <Rd>, <addressing_mode>
 *
 * Usage
 *
 * - Using the PC as the base register allows PC-relative addressing, which
 *   facilitates position-independent code. Combined with a suitable 
 *   addressing mode, LDR allows 32-bit memory data to be loaded into a
 *   general-purpose register where its value can be manipulated. If the
 *   destination register is the PC, this instruction loads a 32-bit address
 *   from memory and branches to that address.
 * - To synthesize a Branch with Link, precede the LDR instruction with 
 *   MOV LR, PC.
 */

static int debug_ldr(void)
{
	unsigned long addr = 0x80004000;
	unsigned long R0 = 0; /* 1st page dirent */
	unsigned long R1 = addr + 1024 * 8; /* 1st kernel page dirent */
	unsigned long R2 = addr + 1025 * 8; /* 2nd kernel page dirent */
	unsigned long R0_val, R1_val, R2_val;

	/* 
	 * Load memory into Register 
	 *   LDR Rn, [R1] --> Load R1 memory value into Rn register.
	 */
	__asm__ volatile ("ldr %0, [%1]" : "=r" (R1_val) : "r" (R1));
	__asm__ volatile ("ldr %0, [%1]" : "=r" (R2_val) : "r" (R2));

	printk("Memory[%#lx] %#lx\n", R1, R1_val);
	printk("Memory[%#lx] %#lx\n", R2, R2_val);

	/*
	 * Load R1+R2 memory to Register
	 *   LDR Rn, [R1, R2] --> Load memory value into Rn register that 
	 *                        memory address is (R1 + R2).
	 */
	__asm__ volatile ("ldr %0, [%1, %2]" 
			: "=r" (R0_val) : "r" (R0), "r" (R1));
	printk("Memory[%#lx + %#lx] %#lx\n", R0, R1, R0_val);

	/*
	 * Load R1+#1 memory to Register
	 *   LDR Rn, [R1, #8] --> Load memory value into Rn register that
	 *                        memory address is (R1 + #8)
	 */
	__asm__ volatile ("ldr %0, [%1, #8]"
			: "=r" (R1_val) : "r" (R1));
	printk("Memory[%#lx + 8] %#lx\n", R1, R1_val);

	/*
	 * Load R1+R2 memory to Register and load R1+R2 address into R1.
	 *   LDR Rn, [R1 + R2]! --> Load memory value into Rn Reister that
	 *                         memory address is (R1 + R2), and move
	 *                         (R1 + R2) address into R1.
	 */
	R0 = 0;
	__asm__ volatile ("ldr %0, [%1, %2]!"
			: "=r" (R1_val) : "r" (R0), "r" (R1));
	printk("Memory[0 + %#lx] %#lx --> R0: %#lx\n", R1, R1_val, R0);

	/* 
	 * Load R0+#0 memory to Register and load R0+#0 address into R0.
	 *   LDR Rn, [R1 + #0]! --> Load memory value into Rn Register that
	 *                          memory address is  (R1 + #0), and move
	 *                          (R0 + #0) address into R0.
	 */
	R0 = 0x80006000;
	__asm__ volatile ("ldr %0, [%1, #8]!"
			: "=r" (R0_val) : "r" (R0));
	printk("Memory[8 + 0x80006000] %#lx --> R0: %#lx\n", R0_val, R0);


	/*
	 * Load R1 memory to Register and load R1+R2 address into R1.
	 *   LDR Rn, [R1], R2 --> Load R1 memory value into Rn Register and
	 *                        move (R1 + R2) address into R1.
	 */
	R0 = 0x80006000;
	R1 = 8;
	__asm__ volatile ("ldr %0, [%1], %2" : "=r" (R0_val) 
					     : "r" (R0), "r" (R1));
	printk("Memory[0x80006000] %#lx --> R0: %#lx\n", R0_val, R0);

	/*
	 * Load (R1+R2<<N) memory to Register and move (R1+R2<<N) into R1
	 *   LDR Rn, [R1, R2, LSL #3]!
	 */
	R0 = 0x80006000;
	R1 = 1;
	__asm__ volatile ("ldr %0, [%1, %2, lsl #3]!" : "=r" (R0_val)
					: "r" (R0), "r" (R1));
	printk("Memory[0x80006000 + %#lx << 3]: %#lx --> R0: %#lx\n",
			R1, R0_val, R0);

	return 0;
}
device_initcall(debug_ldr);
