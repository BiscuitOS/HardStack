/*
 * sys_truncate64 in C
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
/* __NR_truncate64 */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

#ifndef __NR_truncate64
#define __NR_truncate64	193
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_truncate64 helper\n");
	printf("Usage:\n");
	printf("      %s <-p pathname> <-t tuncate_size>\n", program_name);
	printf("\n");
	printf("\t-p\t--path\tThe full path for truncate.\n");
	printf("\t-t\t--size\tThe size for truncate.\n");
	printf("\ne.g:\n");
	printf("%s -p BiscuitOS_file -t 0x100\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *path = NULL;
	long size = -1;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hp:t:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "path", required_argument, NULL, 'p'},
		{ "truncate", required_argument, NULL, 't'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'p': /* Path */
			path = optarg;
			break;
		case 't': /* truncate size */
			sscanf(optarg, "%lx", &size);
			break;
		default:
			abort();
		}
	}

	if (hflags || !path || (size < 0)) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_truncate64
	 *
	 *    SYSCALL_DEFINE2(truncate64,
	 *                    const char __user *, path,
	 *                    loff_t, length)
	 */
	syscall(__NR_truncate64, path, size);

	return 0;
}
