/*
 * sys_ustat in C
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
/* __NR_ustat */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
/* makedev */
#include <sys/sysmacros.h>
/* struct ustat */
#include <ustat.h>

/* Architecture defined */
#ifndef __NR_ustat
#define __NR_ustat	62
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_ustat helper\n");
	printf("Usage:\n");
	printf("      %s <-m MAJOR> <-n MINOR>\n", program_name);
	printf("\n");
	printf("\t-m\t--major\tThe MAJOR for mount device.\n");
	printf("\t-n\t--minor\tThe MINOR for mount device.\n");
	printf("\ne.g:\n");
	printf("%s -m 254 -n 0\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	int major = 0, minor = 0;
	struct ustat ubuf;
	int ret;
	opterr = 0;

	/* options */
	const char *short_opts = "hm:n:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "major", required_argument, NULL, 'm'},
		{ "minor", required_argument, NULL, 'n'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'm': /* MAJOR */
			sscanf(optarg, "%d", &major);
			break;
		case 'n': /* MINOR */
			sscanf(optarg, "%d", &minor);
			break;
		default:
			abort();
		}
	}

	if (hflags || (!major && !minor)) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_ustat
	 *
	 *    SYSCALL_DEFINE2(ustat,
	 *                    unsigned, dev,
	 *                    struct ustat __user *, ubuf)
	 */
	ret = syscall(__NR_ustat, makedev(minor, major), &ubuf);
	if (ret < 0)
		printf("__NR_ustat failed.\n");
	else
		printf("Filesystem: %s\n", ubuf.f_fname);

	return 0;
}
