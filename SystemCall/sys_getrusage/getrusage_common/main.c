/*
 * sys_getrusage in C
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
/* __NR_getrusage */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
#include <sys/resource.h>

/* Architecture defined */
#ifndef __NR_getrusage
#define __NR_getrusage	77
#endif

/* Architecture flags */
#ifndef RUSAGE_SELF
#define RUSAGE_SELF	0	/* The calling process.  */
#endif
#ifndef RUSAGE_CHILDREN
#define RUSAGE_CHILDREN	-1	/* All of its terminated child processes. */
#endif
#ifndef RUSAGE_THREAD
#define RUSAGE_THREAD	1	/* The calling thread.  */
#endif
#ifndef RUSAGE_LWP
#define RUSAGE_LWP	RUSAGE_THREAD	/* The calling thread.  */
#endif


static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_getrusage helper\n");
	printf("Usage:\n");
	printf("      %s <-i item>\n", program_name);
	printf("\n");
	printf("\t-i\t--item\tThe item for searching.\n");
	printf("\t\t\tRUSAGE_SELF       The calling process\n");
	printf("\t\t\tRUSAGE_CHILDREN   All of its terminated child processes\n");
	printf("\t\t\tRUSAGE_THREAD     The calling thread\n");
	printf("\t\t\tRUSAGE_LWP        Name for the same functionality on Solaris\n");
	printf("\ne.g:\n");
	printf("%s -i RUSAGE_SELF\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *res = NULL;
	int c, hflags = 0;
	opterr = 0;
	struct rusage rusage;
	int who;
	int ret;

	/* options */
	const char *short_opts = "hi:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "item", required_argument, NULL, 'i'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'i': /* item */
			res = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !res) {
		usage(argv[0]);
		return 0;
	}

	/* parse item argument */
	if (strstr(res, "RUSAGE_SELF"))
		who = RUSAGE_SELF;
	else if (strstr(res, "RUSAGE_CHILDREN"))
		who = RUSAGE_CHILDREN;
	else if (strstr(res, "RUSAGE_THREAD"))
		who = RUSAGE_THREAD;
	else if (strstr(res, "RUSAGE_LWP"))
		who = RUSAGE_LWP;

	/*
	 * sys_getrusage
	 *
	 *    SYSCALL_DEFINE2(getrusage,
	 *                    int, who,
	 *                    struct rusage __user *, ru)
	 */
	ret = syscall(__NR_getrusage, who, &rusage);
	if (ret < 0) {
		printf("getrusage failed\n");
		return -1;
	}
	printf("The integral shared memory size: %#lx\n", rusage.ru_ixrss);
	
	return 0;
}
