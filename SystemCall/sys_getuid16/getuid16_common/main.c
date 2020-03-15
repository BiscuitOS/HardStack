/*
 * sys_getuid16 in C
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
/* __NR_setuid16/__NR_getuid16 */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_setuid16
#define __NR_setuid16	23
#endif
#ifndef __NR_getuid16
#define __NR_getuid16	24
#endif

/* legacy data struct type */
typedef unsigned int old_uid_t;

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_getuid16 helper\n");
	printf("Usage:\n");
	printf("      %s <-i uid>\n", program_name);
	printf("\n");
	printf("\t-i\t--uid\tThe uid for setting.\n");
	printf("\ne.g:\n");
	printf("%s -i 0\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	old_uid_t uid = 0, ruid = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hi:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "uid", required_argument, NULL, 'i'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'i': /* uid */
			sscanf(optarg, "%d", &uid);
			break;
		default:
			abort();
		}
	}

	if (hflags || !uid) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_setuid16
	 *
	 *    SYSCALL_DEFINE1(setuid16,
	 *                    old_uid_t, uid)
	 */
	syscall(__NR_setuid16, uid);

	/*
	 * sys_getuid16
	 *
	 *    SYSCALL_DEFINE0(getuid16)
	 */
	ruid = syscall(__NR_getuid16);
	printf("getuid16: %#x\n", ruid);

	return 0;
}
