/*
 * sys_access in C
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
/* access flags */
#include <fcntl.h>
/* __NR_access */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_access
#define __NR_access	33
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_access helper\n");
	printf("Usage:\n");
	printf("      %s <-p pathname> <-m mode>\n", program_name);
	printf("\n");
	printf("\t-p\t--path\tThe full path for testing.\n");
	printf("\t-m\t--mode\tThe test mode.\n");
	printf("\t\t\tR_OK   Test for read permission\n");
	printf("\t\t\tW_OK   Test for write permission\n");
	printf("\t\t\tX_OK   Test for execute permission\n");
	printf("\t\t\tF_OK   Test for existence\n");
	printf("\ne.g:\n");
	printf("%s -p BiscuitOS_file -m F_OK,R_OK\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *path = NULL;
	char *mode = NULL;
	int omode = 0;
	int c, hflags = 0;
	opterr = 0;
	int permission = 0;

	/* options */
	const char *short_opts = "hp:m:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "path", required_argument, NULL, 'p'},
		{ "mode", required_argument, NULL, 'm'},
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
		case 'm': /* mode */
			mode = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !path || !mode) {
		usage(argv[0]);
		return 0;
	}
	
	if (mode) {
		if (strstr(mode, "R_OK"))
			omode |= R_OK;
		if (strstr(mode, "W_OK"))
			omode |= W_OK;
		if (strstr(mode, "X_OK"))
			omode |= X_OK;
		if (strstr(mode, "F_OK"))
			omode |= F_OK;
	}

	/*
	 * sys_access
	 *
	 *    SYSCALL_DEFINE2(access,
	 *                    const char __user *, filename,
	 *                    int, mode)
	 */
	permission = syscall(__NR_access, path, omode);
	printf("Permission: %s\n", permission ? "true" : "false");

	return 0;
}
