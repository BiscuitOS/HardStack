/*
 * sys_getgid16 in C
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
/* __NR_setgid16/__NR_getgid16 */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_setgid16
#define __NR_setgid16	46
#endif
#ifndef __NR_getgid16
#define __NR_getgid16	47
#endif

typedef unsigned int old_gid_t;

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_getgid16 helper\n");
	printf("Usage:\n");
	printf("      %s <-g gid>\n", program_name);
	printf("\n");
	printf("\t-g\t--gid\tThe GID for setting.\n");
	printf("\ne.g:\n");
	printf("%s -g 23\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	old_gid_t gid = 0, current_gid = 0;
	opterr = 0;
	int ret;

	/* options */
	const char *short_opts = "hg:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "gid", required_argument, NULL, 'g'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'g': /* GID */
			sscanf(optarg, "%d", &gid);
			break;
		default:
			abort();
		}
	}

	if (hflags || !gid) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_setgid16
	 *
	 *    SYSCALL_DEFINE1(setgid16,
	 *                    old_gid_t, gid)
	 */
	ret = syscall(__NR_setgid16, gid);
	if (ret < 0) {
		printf("setgid16() failed!\n");
		return -1;
	}

	/*
	 * sys_getgid16
	 *
	 *    SYSCALL_DEFINE0(getgid16)
	 */
	current_gid = syscall(__NR_getgid16);
	if (current_gid < 0) {
		printf("getgid16() failed\n");
		return -1;
	} else
		printf("Current GID: %d\n", current_gid);

	return 0;
}
