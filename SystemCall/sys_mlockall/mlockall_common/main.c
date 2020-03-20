/*
 * sys_mlockall in C
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
/* __NR_mlockall */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
#include <sys/mman.h>

/* Architecture defined */
#ifndef __NR_mlockall
#define __NR_mlockall	152
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_mlock helper\n");
	printf("Usage:\n");
	printf("      %s <-f flags>\n", program_name);
	printf("\t-f\t--flags\tThe flags for mlockall.\n");
	printf("\t\t\tMCL_CURRENT    lock all current mappings\n");
	printf("\t\t\tMCL_FUTURE     lock all future mappings\n");
	printf("\t\t\tMCL_ONFAULT    lock all pages that are faulted in\n");
	printf("\ne.g:\n");
	printf("%s -f MCL_CURRENT\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	char *memory;
	char *flags_str;
	int flags;
	const int alloc_size = 32 * 1024 * 1024;
	int index;
	opterr = 0;

	/* options */
	const char *short_opts = "hf:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "flags", required_argument, NULL, 'f'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'f': /* mlockall flags */
			flags_str = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !flags_str) {
		usage(argv[0]);
		return 0;
	}

	if (strstr(flags_str, "MCL_CURRENT"))
		flags = MCL_CURRENT;
	else if (strstr(flags_str, "MCL_FUTURE"))
		flags = MCL_FUTURE;
	else if (strstr(flags_str, "MCL_ONFAULT"))
		flags = MCL_ONFAULT;

	/*
	 * sys_mlockall
	 *
	 *    SYSCALL_DEFINE1(mlockall,
	 *                    int, flags)
	 */
	if (syscall(__NR_mlockall, flags) < 0) {
		printf("mlockall failed.\n");
		return -1;
	}

	/*
	 * sys_munlockall
	 *
	 *    SYSCALL_DEFINE0(munlockall)
	 */
	if (syscall(__NR_munlockall) < 0) {
		printf("munlockall failed.\n");
		return -1;
	}

	return 0;
}
