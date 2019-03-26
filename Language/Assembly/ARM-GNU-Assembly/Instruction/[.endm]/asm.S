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
 * .endm 
 *
 *   Mark the end of a macro definition
 */
@ No argument
.macro BS_nop
	mov r5, r5		
	mov r5, r5		
	mov r5, r5		
.endm

@ A argument
.macro BS_f2 reg1
	mov \reg1, r5
	mov \reg1, r5
	mov \reg1, r5
.endm

@ Two arguments
.macro BS_f3 reg1 reg2
	mov \reg1, \reg2
	mov \reg1, \reg2
	mov \reg1, \reg2
	mov \reg1, \reg2
.endm

ENTRY(BS_func)
	BS_nop
	BS_f2 r5
	BS_f3 r2 r2
	mov r0, r0
	mov r1, r1
	mov r1, r1
	ret     lr
ENDPROC(BS_func)
