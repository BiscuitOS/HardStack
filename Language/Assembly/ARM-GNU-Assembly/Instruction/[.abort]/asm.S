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
 * .abort
 *
 * This directive stops the assembly immediately. It is for compatibility 
 * with other assemblers. The original idea was that the assembly language 
 * source would be piped into the assembler. If the sender of the source 
 * quit, it could use this directive to tell as to quit also. One day .abort 
 * will not be supported.
 *
 * .ABORT is accepted as an alternate spelling of .abort.
 */

ENTRY(BS_func)
	.abort
	mov r0, r0
	ret     lr
ENDPROC(BS_func)
