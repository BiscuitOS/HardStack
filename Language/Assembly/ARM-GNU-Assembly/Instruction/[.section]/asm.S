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
 * .section name
 *
 *   Use the .section directive to assemble the following code into a 
 *   section named name. This directive is only supported for targets 
 *   that actually support arbitrarily named sections; on a.out targets, 
 *   for example, it is not accepted, even with a standard a.out section
 *   name.
 *
 *   For COFF targets, the .section directive is used in one of the 
 *   following ways:
 *
 *     .section name[, "flags"]
 *     .section name[, subsegment]
 *
 *   If the optional argument is quoted, it is taken as flags to use for
 *   the section. Each flag is a single character. The following flags are
 *   recognized:
 *
 *     b bss section (uninitialized data)
 *     n section is not loaded
 *     w writable section
 *     d data section
 *     r read-only section
 *     x executable section
 *
 *   If no flags are specified, the default flags depend upon the section
 *   name. If the section name is not recognized, the default will be for 
 *   the section to be loaded and writable. If the optional argument to 
 *   the .section directive is not quoted, it is taken as a sub-segment 
 *   number. For ELF targets, the .section directive is used like this:
 *
 *     .section name[, "flags"[, @type]]
 *
 *   The optional flags argument is a quoted string which may contain any
 *   combintion of the following characters:
 *
 *     a section is allocatable
 *     w section is writable
 *     x section is executable
 *   
 *   The optional type argument may contain one of the following constants:
 *
 *     @progbits       section contains data46
 *     @nobits         Using as section does not contain data (i.e., 
 *                     section only occupies space)
 *
 *   If no flags are specified, the default flags depend upon the section
 *   name. If the section name is not recognized, the default will be for 
 *   the section to have none of the above flags: it will not be allocated
 *   in memory, nor writable, nor executable. The section will contain data.
 *   For ELF targets, the assembler supports another type of .section 
 *   directive for compatibility with the Solaris assembler:
 *
 *     .section "name"[, flags...]
 *
 *   Note that the section name is quoted. There may be a sequence of comma
 *   separated flags:
 *
 *     #alloc          section is allocatable
 *     #write          section is writable
 *     #execinstr      section is executable
 */
	.section ".start", #alloc, #execinstr

ENTRY(BS_func)
	mov r0, r0
	mov r1, r1
	mov r1, r1
	ret     lr
ENDPROC(BS_func)
