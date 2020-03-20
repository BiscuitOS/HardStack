/*
 * sys_setregid in C
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
/* __NR_setregid */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_setregid
#define __NR_setregid	204
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_setregid helper\n");
	printf("Usage:\n");
	printf("      %s <-r rgid> <-e egid>\n", program_name);
	printf("\n");
	printf("\t-r\t--ruid\tThe real gid for current process.\n");
	printf("\t-e\t--euid\tThe effect gid for current process.\n");
	printf("\ne.g:\n");
	printf("%s -r 269 -e 678\n\n", program_name);
}

int main(int argc, char *argv[])
{
	gid_t rgid = 0, egid = 0;
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
			sscanf(optarg, "%d", &rgid);
			break;
		case 'e': /* Effect UID */
			sscanf(optarg, "%d", &egid);
			break;
		default:
			abort();
		}
	}

	if (hflags || !rgid || !egid) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_setregid
	 *
	 *    SYSCALL_DEFINE2(setregid,
	 *                    gid_t, rgid,
	 *                    gid_t, egid)
	 */
	syscall(__NR_setregid, rgid, egid);
	return 0;
}
