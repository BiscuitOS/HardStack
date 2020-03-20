/*
 * sys_setresuid16 in C
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
/* __NR_setresuid16/__NR_getresuid16 */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_setresuid16
#define __NR_setresuid16	164
#endif
#ifndef __NR_getresuid16
#define __NR_getresuid16	165
#endif

typedef unsigned int old_uid_t;

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_setresuid16 helper\n");
	printf("Usage:\n");
	printf("      %s <-r ruid> <-e euid> <-s suid>\n", program_name);
	printf("\n");
	printf("\t-r\t--ruid\tThe RUID.\n");
	printf("\t-e\t--euid\tThe EUID.\n");
	printf("\t-s\t--suid\tThe SUID.\n");
	printf("\ne.g:\n");
	printf("%s -r 89 -e 86 -s 78\n\n", program_name);
}

int main(int argc, char *argv[])
{
	old_uid_t ruid, euid, suid;
	old_uid_t gruid, geuid, gsuid;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hr:e:s:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "ruid", required_argument, NULL, 'r'},
		{ "euid", required_argument, NULL, 'e'},
		{ "suid", required_argument, NULL, 's'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'r': /* RUID */
			sscanf(optarg, "%d", &ruid);
			break;
		case 'e': /* EUID */
			sscanf(optarg, "%d", &euid);
			break;
		case 's': /* SUID */
			sscanf(optarg, "%d", &suid);
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
	 * sys_setresuid16
	 *
	 *    SYSCALL_DEFINE3(setresuid16,
	 *                    old_uid_t, ruid,
	 *                    old_uid_t, euid,
	 *                    old_uid_t, suid)
	 */
	syscall(__NR_setresuid16, ruid, euid, suid);

	/*
	 * sys_getresuid16
	 *
	 *    SYSCALL_DEFINE3(getresuid16,
	 *                    old_uid_t __user *, ruidp,
	 *                    old_uid_t __user *, euidp,
	 *                    old_uid_t __user *, suidp)
	 */
	syscall(__NR_getresuid16, &gruid, &geuid, &gsuid);
	printf("RUID: %d\nEUID: %d\nSUID: %d\n", gruid, geuid, gsuid);

	return 0;
}
