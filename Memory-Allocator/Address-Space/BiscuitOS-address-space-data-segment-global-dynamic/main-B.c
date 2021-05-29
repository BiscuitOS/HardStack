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

int main()
{
	extern char __executable_start[];
	extern char edata[];
	extern char etext[];
	extern char end[];

	printf("***************************************************************\n");
	printf("+---+-------+-------+------+--+------+-+------+--------------+\n");
	printf("|   |       |       |      |  |      | |      |              |\n");
	printf("|   | .text | .data | .bss |  | Heap | | Mmap |        Stack |\n");
	printf("|   |       |       |      |  |      | |      |              |\n");
	printf("+---+-------+-------+------+--+------+-+------+--------------+\n");
	printf("0                                                            TASK_SIZE\n");
	printf("Executable start: %#016lx\n", (unsigned long)__executable_start);
	printf("Code Range:       %#016lx -- %#016lx\n", 
						(unsigned long)__executable_start,
						(unsigned long)etext);
	printf("Data Range:       %#016lx -- %#016lx\n", (unsigned long)etext,
						(unsigned long)edata);
	printf("BSS  Range:       %#016lx -- %#016lx\n", (unsigned long)edata,
						(unsigned long)end);
	printf("***************************************************************\n");

	/* Direct ref */
	BiscuitOS_other_global_uninit_variable   = 88526;
	BiscuitOS_other_global_init_variable     = 88527;

	printf("B: BiscuitOS_other_global_uninit_variable: %#lx - %ld\n",
		(unsigned long)&BiscuitOS_other_global_uninit_variable,
				BiscuitOS_other_global_uninit_variable);
	printf("B: BiscuitOS_other_global_init_variable:   %#lx - %ld\n",
		(unsigned long)&BiscuitOS_other_global_init_variable,
				BiscuitOS_other_global_init_variable);

	/* only debug */
	printf("Main B PID %ld\n", (long)getpid());
	sleep(-1);
	return 0;
}
