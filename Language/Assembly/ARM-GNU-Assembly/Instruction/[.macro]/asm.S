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
 * .macro macname
 *
 * .macro macname macargs . . .
 *
 *   Begin the definition of a macro called macname. If your macro 
 *   definition requires arguments, specify their names after the 
 *   macro name, separated by commas or spaces. You can supply a 
 *   default value for any macro argument by following the name with 
 *   ‘=deflt’. For example, these are all valid .macro statements:
 *
 *     .macro comm           Begin the definition of a macro called comm, 
 *                           which takes no arguments.
 *     .macro plus1 p, p1
 *     .macro plus1 p p1     Either statement begins the definition of a 
 *                           macro called plus1, which takes two arguments; 
 *                           within the macro definition, write ‘\p’ or 
 *                           ‘\p1’ to evaluate the arguments.
 *     .macro reserve_str p1=0 p2
 *                           Begin the definition of a macro called 
 *                           reserve_str, with two arguments. The first 
 *                           argument has a default value, but not the 
 *                           second. After the definition is complete, 
 *                           you can call the macro either as ‘reserve_str 
 *                           a,b’ (with ‘\p1’ evaluating to a and ‘\p2’ 
 *                           evaluating to b), or as ‘reserve_str ,b’ (with 
 *                           ‘\p1’ evaluating as the default, in this case 
 *                           ‘0’, and ‘\p2’ evaluating to b).
 *
 *   When you call a macro, you can specify the argument values either by 
 *   position, or by keyword. For example, ‘sum 9,17’ is equivalent to 
 *   ‘sum to=17, from=9’.
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
