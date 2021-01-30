/*
 * Intel EFLAGS Register
 *
 * EFLAGS Register
 *
 *  31                                                        9 8 7 6 5 4 3 2 1 0
 * +-----------------------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                   | |V|V| | | | | | | | | | | | | | | | | | |
 * |        Reserved (set to 0)        |I|I|I|A|V|R|0|N| |O|D|I|T|S|Z|0|A|0|P|1|C|
 * |                                   |D|P|F|C|M|F| |T| |F|F|F|F|F|F| |F| |F| |F|
 * +-----------------------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                                      | | | | | |   | | | | | | | |   |   |   |
 *                                      | | | | | |   | | | | | | | |   |   |   |
 *                                      | | | | | |   | | | | | | | |   |   |   |
 * ID  -- Identification Flag ----------o | | | | |   | | | | | | | |   |   |   |
 * VIP -- Virtual Interrupt Pending ------o | | | |   | | | | | | | |   |   |   |
 * VIF -- Virtual Interrupt Flag -----------o | | |   | | | | | | | |   |   |   |
 * AC  -- Alignment Check/ Access Control ----o | |   | | | | | | | |   |   |   |
 * VM  -- Virtual-8086 Mode --------------------o |   | | | | | | | |   |   |   |
 * RF  -- Resume Flag ----------------------------o   | | | | | | | |   |   |   |
 * NT  -- Nested Task Flag ---------------------------o | | | | | | |   |   |   |
 * IOPL - I/O Privillege level -------------------------o | | | | | |   |   |   |
 * OF  -- Overflow Flag ----------------------------------o | | | | |   |   |   |
 * DF  -- Direction Flag -----------------------------------o | | | |   |   |   |
 * IF  -- Interrupt Enable Flag ------------------------------o | | |   |   |   |
 * TF  -- Trap Flag --------------------------------------------o | |   |   |   |
 * SF  -- Sign Flag ----------------------------------------------o |   |   |   |
 * ZF  -- Zero Flag ------------------------------------------------o   |   |   |
 * AF  -- Auxiliary Carry Flag -----------------------------------------o   |   |
 * PF  -- Parity Flag ------------------------------------------------------o   |
 * CF  -- Carry Flag -----------------------------------------------------------o
 *
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
/* Header */
#include <asm/msr.h>
#include <asm/msr-index.h>

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned int eflags;

	/* Push eflags into stack */
	__asm__ volatile ("pushf\n\r"
			  "popl %%eax"
			  : "=a" (eflags));

	printk("EFLAGS %#lx\n", eflags);

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Intel Register on BiscuitOS");
