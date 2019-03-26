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
 * .thumb 
 * 
 *   This performs the same action as .code 16.
 *
 * .code 16
 *
 *   This directive selects the instruction set being generated. The 
 *   value 16 selects Thumb.
 */

#ifdef CONFIG_THUMB2_KERNEL
	.thumb
#endif
ENTRY(BS_func)
	mov r0, r0
	mov r1, r1
	mov r1, r1
	ret     lr
ENDPROC(BS_func)
