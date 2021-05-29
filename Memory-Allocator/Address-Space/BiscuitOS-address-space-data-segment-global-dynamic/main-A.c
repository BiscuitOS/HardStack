/*
 * Process Address Space: Data Segment with Gloabl variable
 *
 * (C) 2021.05.01 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** Define on other file **/
extern unsigned long BiscuitOS_other_global_uninit_variable;
extern unsigned long BiscuitOS_other_global_init_variable;
unsigned long BiscuitOS_current_global_uninit_variable;
unsigned long BiscuitOS_current_global_init_variable = 52088;

int main()
{
	/* Direct ref */
	BiscuitOS_other_global_uninit_variable   = 88520;
	BiscuitOS_other_global_init_variable     = 88521;

	BiscuitOS_current_global_uninit_variable = 88522;
	BiscuitOS_current_global_init_variable   = 88523;

	/* only debug */
	printf("Main A PID %ld\n", (long)getpid());
	sleep(-1);
	return 0;
}
