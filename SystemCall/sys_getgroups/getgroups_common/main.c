/*
 * sys_getgroups in C
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
/* __NR_setgroups/__NR_getgroups */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_setgroups
#define __NR_setgroups	206
#endif
#ifndef __NR_getgroups
#define __NR_getgroups	205
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_getgroups helper\n");
	printf("Usage:\n");
	printf("      %s\n", program_name);
	printf("\n");
	printf("\ne.g:\n");
	printf("%s\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	opterr = 0;
	gid_t list[128];
	int x, index;

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

	/*
	 * sys_getgroups
	 *
	 *    SYSCALL_DEFINE2(getgroups,
	 *                    int, gidsetsize,
	 *                    gid_t __user *, grouplist)
	 */
	x = syscall(__NR_getgroups, 0, list);
	syscall(__NR_getgroups, x, list);
	for (index = 0; index < x; index++)
		printf("Group [%d] GID: %d\n", index, list[index]);

	return 0;
}
