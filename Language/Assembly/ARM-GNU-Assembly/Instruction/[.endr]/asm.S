/*
 * GNU ARM assembly
 *
 * (C) 2019.03.26 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/v7m.h>

#include "efi-header.S"

/*
 * .endr
 *
 *   Repeat the sequence of lines between the .rept directive and the next 
 *   .endr directive count times.
 *
 *   For example, assembling
 *
 *     .rept 3
 *     .long 3
 *     .endr
 *
 *   is equivalent to assembling
 *
 *     .long 3
 *     .long 3
 *     .long 3
 */

ENTRY(BS_func)
	.rept 6
	.long 5
	.endr
	mov r0, r0
	mov r1, r1
	mov r1, r1
	ret     lr
ENDPROC(BS_func)
