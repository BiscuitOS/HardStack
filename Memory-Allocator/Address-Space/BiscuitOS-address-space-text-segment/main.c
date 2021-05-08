/*
 * Process Address Space: Text Segment
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

int main()
{
	extern char __executable_start[];
	extern char edata[];
	extern char etext[];
	extern char end[];

	printf("***************************************************************\n");
	printf("Code Segment: .text Describe\n\n");
	printf("+---+-------+-------+------+--+------+-+------+--------------+\n");
	printf("|   |       |       |      |  |      | |      |              |\n");
	printf("|   | .text | .data | .bss |  | Heap | | Mmap |        Stack |\n");
	printf("|   |       |       |      |  |      | |      |              |\n");
	printf("+---+-------+-------+------+--+------+-+------+--------------+\n");
	printf("0                                                            TASK_SIZE\n");
	printf("Executable start: %#08lx\n", (unsigned long)__executable_start);
	printf("Code Range:       %#08lx -- %#08lx\n", (unsigned long)__executable_start,
	                                               (unsigned long)etext);
	printf("Data Range:       %#08lx -- %#08lx\n", (unsigned long)etext,
	                                               (unsigned long)edata);
	printf("BSS  Range:       %#08lx -- %#08lx\n", (unsigned long)edata,
	                                               (unsigned long)end);
	printf("***************************************************************\n");

	return 0;
}
