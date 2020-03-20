/*
 * sys_mlock in C
 *
 * (C) 2020.03.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
/* __NR_open/__NR_mlock */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
#include <malloc.h>

/* Architecture defined */
#ifndef __NR_mlock
#define __NR_mlock	150
#endif

#define PAGE_SIZE	(1 << 12)

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_mlock helper\n");
	printf("Usage:\n");
	printf("      %s\n", program_name);
	printf("\ne.g:\n");
	printf("%s -p BiscuitOS_file -f O_RDWR,O_CREAT "
			"-m S_IRUSR,S_IRGRP\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	char *memory;
	const int alloc_size = 32 * 1024 * 1024;
	int index;
	opterr = 0;

	/* options */
	const char *short_opts = "h";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		default:
			abort();
		}
	}

	if (hflags) {
		usage(argv[0]);
		return 0;
	}

	memory = malloc(alloc_size);

	/*
	 * sys_mlock
	 *
	 *    SYSCALL_DEFINE2(mlock,
	 *                    unsigned long, start,
	 *                    size_t, len)
	 */
	if (syscall(__NR_mlock, memory, alloc_size) < 0) {
		printf("mlock failed.\n");
		free(memory);
		return -1;
	}

	for (index = 0; index < alloc_size; index += PAGE_SIZE)
		memory[index] = 0;

	/*
	 * sys_munlock
	 *
	 *    SYSCALL_DEFINE2(munlock,
	 *                    unsigned long, start,
	 *                    size_t, len)
	 */
	if (syscall(__NR_munlock, memory, alloc_size) < 0) {
		printf("munlock failed.\n");
		free(memory);
		return -1;
	}

	free(memory);
	return 0;
}
