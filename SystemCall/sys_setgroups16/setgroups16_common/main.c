/*
 * sys_setgroups16 in C
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
/* __NR_setgroups16/__NR_getgroups16 */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_setgroups16
#define __NR_setgroups16	81
#endif
#ifndef __NR_getgroups16
#define __NR_getgroups16	80
#endif

typedef unsigned int old_gid_t;

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_setgroups helper\n");
	printf("Usage:\n");
	printf("      %s <-g gid>\n", program_name);
	printf("\t-g\t--gid\tThe GID for group.\n");
	printf("\n");
	printf("\ne.g:\n");
	printf("%s -g 24\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	opterr = 0;
	old_gid_t gid, rgid;

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
		case 'g':
			sscanf(optarg, "%d", &gid);
			break;
		default:
			abort();
		}
	}

	if (hflags) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_setgroups16
	 *
	 *    SYSCALL_DEFINE2(setgroups16,
	 *                    int, gidsetsize,
	 *                    old_gid_t __user *, grouplist)
	 */
	syscall(__NR_setgroups16, 1, &gid);

	/*
	 * sys_getgroups16
	 *
	 *    SYSCALL_DEFINE2(getgroups16,
	 *                    int, gidsetsize,
	 *                    old_gid_t __user *, grouplist)
	 */
	syscall(__NR_getgroups16, 1, &rgid);
	printf("Group GID: %d\n", rgid);

	return 0;
}
