// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault ERROR CODE: PF_INSTR
 *
 * (C) 2023.09.03 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef void (*BiscuitOS_func_t)();

int main()
{
	BiscuitOS_func_t func = 0x6000000000;

	/* Execute Case #PF with PF_INSTR */
	func();

	return 0;
}
