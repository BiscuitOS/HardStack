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
 * MSR (Move to Status Register from ARM Register) transfers the value of 
 * a general-purpose register or an immediate constant to the CPSR or the
 * SPSR of the current mode.
 *
 * Syntax
 *   MSR{<cond>} CPSR_<fields>, #<immediate>
 *   MSR{<cond>} CPSR_<fields>, <Rm>
 *   MSR{<cond>} SPSR_<fields>, #<immediate>
 *   MSR{<cond>} SPSR_<fields>, <Rm>
 *
 * 32        27         25     20        16        10        5        0
 * +-+-+-+-+-+----------+-+----+---------+---------+-+-+-+-+-+--------+
 * | | | | | |          | |    |         |         | | | | | |        |
 * |N|Z|C|V|Q| IT[1:0]  |J|    | GE[3:0] | IT[7:2] |E|A|I|F|T| M[4:0] |
 * | | | | | |          | |    |         |         | | | | | |        |
 * +-+-+-+-+-+----------+-+----+---------+---------+-+-+-+-+-+--------+
 * | <-----> |                                       | <-> |
 *  Condition                                       Mask bits
 *    flags
 *
 * Condition flags, bits[31:28]
 *
 *   N,bit[31]       Negative condition flag
 *   Z,bit[30]       Zero condition flag
 *   C,bit[29]       Carry condition flag
 *   V,bit[28]       Overflow condition flag
 *
 * Q,bit[27]
 *   
 *   Cumulative saturation bit.
 *
 * IT[7:0],bits[15:10,26:25]
 *
 *   If-Then execution state bits for the Thumb IT(if-then) instruction.
 *
 * GE[3:0],bits[19:16]
 *
 *   Greater then or Equal flags.
 *
 * E,bit[9]
 *
 *   Endlianness execution state bit.
 *   0     Little-endian operation
 *   1     Big-endian operation
 *
 * Mask bits,bits[8:6]
 *
 *   A,bit[8]        Asynchronous abort mask bit.
 *   I,bit[7]        IRQ mask bit.
 *   F,bit[6]        FIQ mask bit.
 *
 *   The possible value of each bit are:
 *
 *   0               Exception not masked
 *   1               Exception masked
 *
 * T,bit[5] 
 *
 *   Thumb execution state bit.
 *
 * M[4:0],bits[4:0]
 *
 *   Mode field. This field determines the current mode of the processor.
 */

static int debug_msr(void)
{
	unsigned long CPSR;

	/* Read CPSR register */
	__asm__ volatile ("mrs %0, cpsr" : "=r" (CPSR));
	printk("CPSR: %#lx\n", CPSR);
	/* Write data into CPSR register */
	__asm__ volatile ("msr cpsr, %0" :: "r" (CPSR));

	return 0;
}
device_initcall(debug_msr);
