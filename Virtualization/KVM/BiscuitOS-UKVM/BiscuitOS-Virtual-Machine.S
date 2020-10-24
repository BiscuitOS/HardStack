#
# Virtual-Machine OS
#
# (C) 2020.10.24 BuddyZhang1 <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

.globl _BiscuitOS_start
	.code16
	.text

_BiscuitOS_start:
	mov	$0x40, %bx

loop:
	mov	(%bx), %ax
	out	%ax, $0x10
	sub	$'\n', %ax
	jz	_BiscuitOS_start
	inc	%bx
	jmp	loop

	.data
	.org 0x40
	.ascii	"Hello BiscuitOS :)\n"

	.bss
