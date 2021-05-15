/*
 * Process Address Space: data Segment
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

/* Structure */
struct BiscuitOS_struct {
	struct BiscuictOS *parent;
	int number;
};

/* Union */
union BiscuitOS_union {
	unsigned long node;
	int count;
};

/* Enum */
enum BISCUITOS_ENUM {
	BISCUITOS_ZERO,
	BISCUITOS_ONE,
};

/* Static data with uninit  */
static int BiscuitOS_static_uninit_variable;
static int *BiscuitOS_static_uninit_pointer;
static int BiscuitOS_static_uninit_array[10];
static enum BISCUITOS_ENUM BiscuitOS_static_uninit_enum;
static struct BiscuitOS_struct BiscuitOS_static_uninit_struct;
static union BiscuitOS_union BiscuitOS_static_uninit_union;

/* Static data with init  */
static int BiscuitOS_static_init_variable = 88520;
static int *BiscuitOS_static_init_pointer = &BiscuitOS_static_init_variable;
static int BiscuitOS_static_init_array[10] = { 88520, 52088 };
static enum BISCUITOS_ENUM BiscuitOS_static_init_enum = BISCUITOS_ONE;
static struct BiscuitOS_struct BiscuitOS_static_init_struct = { .number = 88520 };
static union BiscuitOS_union BiscuitOS_static_init_union = { .count = 88520 };

/* Globl uninit data: extern */
int BiscuitOS_global_uninit_variable;
int *BiscuitOS_global_uninit_pointer;
int BiscuitOS_global_uninit_array[10];
enum BISCUITOS_ENUM BiscuitOS_global_uninit_enum;
struct BiscuitOS_struct BiscuitOS_global_uninit_struct;
union BiscuitOS_union BiscuitOS_global_uninit_union;

/* Globl init data: extern */
int BiscuitOS_global_init_variable = 88520;
int *BiscuitOS_global_init_pointer = &BiscuitOS_global_init_variable;
int BiscuitOS_global_init_array[10] = { 88520, 52088 };
enum BISCUITOS_ENUM BiscuitOS_global_init_enum = BISCUITOS_ONE;
struct BiscuitOS_struct BiscuitOS_global_init_struct = { .number = 88520 };
union BiscuitOS_union BiscuitOS_global_init_union = { .count = 88520 };

/* Speical variable */
#define BISCUITOS_MACRO		"BiscuitOS"

int main()
{
	extern char __executable_start[];
	extern char edata[];
	extern char etext[];
	extern char end[];
	unsigned long sp, bp;

	/* Bottom for stack */
	asm volatile ("pushl %%ebp\n\r"
		      "popl %0"
		      : "=a" (bp) : : "memory");

	/* Local noninitialize data */
	int BiscuitOS_local_uninit_variable;
	int *BiscuitOS_local_uninit_pointer;
	int BiscuitOS_local_uninit_array[10];
	enum BISCUITOS_ENUM BiscuitOS_local_uninit_enum;
	struct BiscuitOS_struct BiscuitOS_local_uninit_struct;
	union BiscuitOS_union BiscuitOS_local_uninit_union;

	/* Local initialize data */
	int BiscuitOS_local_init_variable = 88520;
	int *BiscuitOS_local_init_pointer = &BiscuitOS_local_init_variable;
	int BiscuitOS_local_init_array[10] = { 88520, 52088 };
	enum BISCUITOS_ENUM BiscuitOS_local_init_enum = BISCUITOS_ONE;
	struct BiscuitOS_struct BiscuitOS_local_init_struct = { .number = 88520 };
	union BiscuitOS_union BiscuitOS_local_init_union = { .count = 88520 };

	/* Local static uninit data */
	static int BiscuitOS_local_static_uninit_variable;
	static int *BiscuitOS_local_static_uninit_pointer;
	static int BiscuitOS_local_static_uninit_array[10];
	static enum BISCUITOS_ENUM BiscuitOS_local_static_uninit_enum;
	static struct BiscuitOS_struct BiscuitOS_local_static_uninit_struct;
	static union BiscuitOS_union BiscuitOS_local_static_uninit_union;

	/* Local static init data */
	static int BiscuitOS_local_static_init_variable = 88520;
	static int *BiscuitOS_local_static_init_pointer = &BiscuitOS_local_static_init_variable;
	static int BiscuitOS_local_static_init_array[10] = { 88520, 52088 };
	static enum BISCUITOS_ENUM BiscuitOS_local_static_init_enum = BISCUITOS_ONE;
	static struct BiscuitOS_struct BiscuitOS_local_static_init_struct = { .number = 88520 };
	static union BiscuitOS_union BiscuitOS_local_static_init_union = { .count = 88520 };

	/* Register variable */
	register int BiscuitOS_register = 88520;
	/* Top for stack */
	asm volatile ("pushl %%esp\n\r"
		      "popl %0"
		      : "=r" (sp) : : "memory");

	printf("***************************************************************\n");
	printf("Code Segment: .text Describe\n\n");
	printf("+---+-------+-------+------+--+------+-+------+--------------+\n");
	printf("|   |       |       |      |  |      | |      |              |\n");
	printf("|   | .text | .data | .bss |  | Heap | | Mmap |        Stack |\n");
	printf("|   |       |       |      |  |      | |      |              |\n");
	printf("+---+-------+-------+------+--+------+-+------+--------------+\n");
	printf("0                                                            TASK_SIZE\n");
	printf("Executable start: %#016lx\n", (unsigned long)__executable_start);
	printf("Code Range:       %#016lx -- %#016lx\n", (unsigned long)__executable_start,
	                                                 (unsigned long)etext);
	printf("Data Range:       %#016lx -- %#016lx\n", (unsigned long)etext,
	                                                 (unsigned long)edata);
	printf("BSS  Range:       %#016lx -- %#016lx\n", (unsigned long)edata,
	                                                 (unsigned long)end);
	printf("Stack:            %#016lx -- %#016lx\n", sp, bp);
	printf("***************************************************************\n");

	/* Static uninit variable */
	printf("Static noninitialize:\n");
	printf("  Variable:   %#lx\n", (unsigned long)&BiscuitOS_static_uninit_variable);
	printf("  Pointer:    %#lx\n", (unsigned long)&BiscuitOS_static_uninit_pointer);
	printf("  Array:      %#lx\n", (unsigned long)BiscuitOS_static_uninit_array);
	printf("  Enum:       %#lx\n", (unsigned long)&BiscuitOS_static_uninit_enum);
	printf("  Struct:     %#lx\n", (unsigned long)&BiscuitOS_static_uninit_struct);
	printf("  Union:      %#lx\n", (unsigned long)&BiscuitOS_static_uninit_union);
	/* Static init variable */
	printf("Static initialize:\n");
	printf("  Variable:   %#lx\n", (unsigned long)&BiscuitOS_static_init_variable);
	printf("  Pointer:    %#lx\n", (unsigned long)&BiscuitOS_static_init_pointer);
	printf("  Array:      %#lx\n", (unsigned long)BiscuitOS_static_init_array);
	printf("  Enum:       %#lx\n", (unsigned long)&BiscuitOS_static_init_enum);
	printf("  Struct:     %#lx\n", (unsigned long)&BiscuitOS_static_init_struct);
	printf("  Union:      %#lx\n", (unsigned long)&BiscuitOS_static_init_union);
	/* Global uninit variable */
	printf("Global noninitialize:\n");
	printf("  Variable:   %#lx\n", (unsigned long)&BiscuitOS_global_uninit_variable);
	printf("  Pointer:    %#lx\n", (unsigned long)&BiscuitOS_global_uninit_pointer);
	printf("  Array:      %#lx\n", (unsigned long)BiscuitOS_global_uninit_array);
	printf("  Enum:       %#lx\n", (unsigned long)&BiscuitOS_global_uninit_enum);
	printf("  Struct:     %#lx\n", (unsigned long)&BiscuitOS_global_uninit_struct);
	printf("  Union:      %#lx\n", (unsigned long)&BiscuitOS_global_uninit_union);
	/* Global init variable */
	printf("Global initialize:\n");
	printf("  Variable:   %#lx\n", (unsigned long)&BiscuitOS_global_init_variable);
	printf("  Pointer:    %#lx\n", (unsigned long)&BiscuitOS_global_init_pointer);
	printf("  Array:      %#lx\n", (unsigned long)BiscuitOS_global_init_array);
	printf("  Enum:       %#lx\n", (unsigned long)&BiscuitOS_global_init_enum);
	printf("  Struct:     %#lx\n", (unsigned long)&BiscuitOS_global_init_struct);
	printf("  Union:      %#lx\n", (unsigned long)&BiscuitOS_global_init_union);
	/* Local uninit variable */
	printf("Local noninitialize:\n");
	printf("  Variable:   %#lx\n", (unsigned long)&BiscuitOS_local_uninit_variable);
	printf("  Pointer:    %#lx\n", (unsigned long)&BiscuitOS_local_uninit_pointer);
	printf("  Array:      %#lx\n", (unsigned long)BiscuitOS_local_uninit_array);
	printf("  Enum:       %#lx\n", (unsigned long)&BiscuitOS_local_uninit_enum);
	printf("  Struct:     %#lx\n", (unsigned long)&BiscuitOS_local_uninit_struct);
	printf("  Union:      %#lx\n", (unsigned long)&BiscuitOS_local_uninit_union);
	/* Local init variable */
	printf("Local initialize:\n");
	printf("  Variable:   %#lx\n", (unsigned long)&BiscuitOS_local_init_variable);
	printf("  Pointer:    %#lx\n", (unsigned long)&BiscuitOS_local_init_pointer);
	printf("  Array:      %#lx\n", (unsigned long)BiscuitOS_local_init_array);
	printf("  Enum:       %#lx\n", (unsigned long)&BiscuitOS_local_init_enum);
	printf("  Struct:     %#lx\n", (unsigned long)&BiscuitOS_local_init_struct);
	printf("  Union:      %#lx\n", (unsigned long)&BiscuitOS_local_init_union);
	/* Local static uninit variable */
	printf("Local static noninitialize:\n");
	printf("  Variable:   %#lx\n", (unsigned long)&BiscuitOS_local_static_uninit_variable);
	printf("  Pointer:    %#lx\n", (unsigned long)&BiscuitOS_local_static_uninit_pointer);
	printf("  Array:      %#lx\n", (unsigned long)BiscuitOS_local_static_uninit_array);
	printf("  Enum:       %#lx\n", (unsigned long)&BiscuitOS_local_static_uninit_enum);
	printf("  Struct:     %#lx\n", (unsigned long)&BiscuitOS_local_static_uninit_struct);
	printf("  Union:      %#lx\n", (unsigned long)&BiscuitOS_local_static_uninit_union);
	/* Static init variable */
	printf("Local static initialize:\n");
	printf("  Variable:   %#lx\n", (unsigned long)&BiscuitOS_local_static_init_variable);
	printf("  Pointer:    %#lx\n", (unsigned long)&BiscuitOS_local_static_init_pointer);
	printf("  Array:      %#lx\n", (unsigned long)BiscuitOS_local_static_init_array);
	printf("  Enum:       %#lx\n", (unsigned long)&BiscuitOS_local_static_init_enum);
	printf("  Struct:     %#lx\n", (unsigned long)&BiscuitOS_local_static_init_struct);
	printf("  Union:      %#lx\n", (unsigned long)&BiscuitOS_local_static_init_union);
	/* Speical variable */
	printf("Special variable\n");
	printf("  Register variable: %#lx\n", (unsigned long)BiscuitOS_register);
	printf("  Macro:             %#lx\n", (unsigned long)&BISCUITOS_MACRO);
	printf("  Constant Strings:  %#lx\n", (unsigned long)"BiscuitOS");

	return 0;
}
