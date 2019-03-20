/*
 * Arm Register
 *
 * (C) 2019.03.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>

/*
 * VMSA CP15 c0 register summary, identification registers
 *
 * Usage:
 *
 *   MCR{<cond>} <coproc>, <opcode_1>, <Rd>, <CRn>, <CRm>{, <opcode_2>}
 *   MRC{<cond>} <coproc>, <opcode_1>, <Rd>, <CRn>, <CRm>{, <opcode_2>}
 *
 * +-----+------+-----+------+-----+-------------------------------------+
 * | CRn | opc1 | CRm | opc2 | R/W | Register                            |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c0  |  0   | RO  | MIDR, Main ID Register              |   
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c0  |  1   | RO  | CTR, Cache Type Register            |   
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c0  |  2   | RO  | TCMTR, TCM Type Register            |   
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c0  |  3   | RO  | TLBTR, TLB Type Register            |   
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c0  |  4/7 | RO  | Aliases of MIDR                     |   
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c0  |  5   | RO  | MPIDR, Multiprocessor Affinity Reg  |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c0  |  6   | RO  | REVIDR, Revision ID Register        |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c1  |  0   | RO  | D_PFR0, Processor Feature Register 0|
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c1  |  1   | RO  | ID_PFR1,Processor Feature Register 1|
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c1  |  2   | RO  | ID_DFR0, Debug Feature Register 0   |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c1  |  3   | RO  | ID_AFR0,Auxiliary Feature Register 0|
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c1  |  4   | RO  | ID_MMFR0, Memory Model Feature Reg 0|
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c1  |  5   | RO  | ID_MMFR1, Memory Model Feature Reg 1|
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c1  |  6   | RO  | ID_MMFR2, Memory Model Feature Reg 2|
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c1  |  7   | RO  | ID_MMFR3, Memory Model Feature Reg 3|
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c2  |  0   | RO  | ID_ISAR0, ISA Feature Register 0    |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c2  |  1   | RO  | ID_ISAR1, ISA Feature Register 1    |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c2  |  2   | RO  | ID_ISAR2, ISA Feature Register 2    |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c2  |  3   | RO  | ID_ISAR3, ISA Feature Register 3    |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c2  |  4   | RO  | ID_ISAR4, ISA Feature Register 4    |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c2  |  5   | RO  | ID_ISAR5, ISA Feature Register 5    |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  | c2  |  6/7 | RO  | Read-As-Zero                        |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   0  |c3-c7|  0-7 | RO  | Read-As-Zero                        |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   1  | c0  |  0   | RO  | CCSIDR, Cache Size ID Registers     |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   1  | c0  |  1   | RO  | CLIDR, Cache Level ID Register      |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   1  | c0  |  7   | RO  | AIDR, Auxiliary ID Register         |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   2  | c0  |  0   | R/W | CSSELR, Cache Size Selection Reg    |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   4  | c0  |  0   | R/W | VPIDR, Virtualization Processor     |
 * |     |      |     |      |     | ID Register                         |
 * +-----+------+-----+------+-----+-------------------------------------+
 * |  c0 |   4  | c0  |  5   | R/W | VMPIDR, Virtualization              |   
 * |     |      |     |      |     | Multiprocessor ID Registe           |   
 * +-----+------+-----+------+-----+-------------------------------------+
 */

static int debug_cp15c0(void)
{
	unsigned long CSS;

	/* Read CSSELR into Rt */
	__asm__ volatile ("mrc p15,2,%0,c0,c0,0" : "=r" (CSS));
	printk("CSSELR: %#lx\n", CSS);

	/* Write Rt into CSSELR */
	__asm__ volatile ("mcr p15,2,%0,c0,c0,0" :: "r" (CSS));

	return 0;
}
device_initcall(debug_cp15c0);
