/*
 * sys_setreuid in C
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
/* __NR_setreuid */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_setreuid
#define __NR_setreuid	203
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_setreuid helper\n");
	printf("Usage:\n");
	printf("      %s <-r ruid> <-e euid>\n", program_name);
	printf("\n");
	printf("\t-r\t--ruid\tThe real uid for current process.\n");
	printf("\t-e\t--euid\tThe effect uid for current process.\n");
	printf("\ne.g:\n");
	printf("%s -r 269 -e 678\n\n", program_name);
}

int main(int argc, char *argv[])
{
	uid_t ruid = 0, euid = 0;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hr:e:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "ruid", required_argument, NULL, 'r'},
		{ "euid", required_argument, NULL, 'e'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'r': /* Real UID */
			sscanf(optarg, "%d", &ruid);
			break;
		case 'e': /* Effect UID */
			sscanf(optarg, "%d", &euid);
			break;
		default:
			abort();
		}
	}

	if (hflags || !ruid || !euid) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_setreuid
	 *
	 *    SYSCALL_DEFINE2(setreuid,
	 *                    uid_t, ruid,
	 *                    uid_t, euid)
	 */
	syscall(__NR_setreuid, ruid, euid);
	return 0;
}
