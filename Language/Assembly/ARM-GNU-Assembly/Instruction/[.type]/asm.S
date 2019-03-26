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
 * .type
 *
 *   This directive, permitted only within .def/.endef pairs, records the 
 *   integer int as the type attribute of a symbol table entry.
 *   
 *   ‘.type’ is associated only with COFF format output; when as is 
 *   configured for b.out output, it accepts this directive but ignores it.
 */

ENTRY(BS_func)
	.type BS_func, #function
ENTRY(BS_OBJ)
	.type BS_OBJ, #object
	mov r0, r0
	ret     lr
ENDPROC(BS_func)
