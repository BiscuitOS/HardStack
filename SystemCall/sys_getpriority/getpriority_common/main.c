/*
 * sys_getpriority in C
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
/* __NR_getpriority */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_getpriority
#define __NR_getpriority	96
#endif

/* Architecture flags */
#ifndef PRIO_PROCESS
#define PRIO_PROCESS	0	/* WHO is a process ID.  */
#endif
#ifndef PRIO_PGRP
#define PRIO_PGRP	1	/* WHO is a process group ID.  */
#endif
#ifndef PRIO_USER
#define PRIO_USER	2	/* WHO is a user ID.  */
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_getpriority helper\n");
	printf("Usage:\n");
	printf("      %s <-w which> <-p who>\n", program_name);
	printf("\n");
	printf("\t-w\t--which\tThe option for searching.\n");
	printf("\t\t\tPRIO_PROCESS     WHO is a process ID\n");
	printf("\t\t\tPRIO_PGRP        WHO is a process group ID\n");
	printf("\t\t\tPRIO_USER        WHO is a user ID\n");
	printf("\t-p\t--pid\tThe process PID.\n");
	printf("\ne.g:\n");
	printf("%s -w PRIO_PROCESS -p 0\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *which = NULL;
	int owhich;
	int who;
	int c, hflags = 0;
	long priority;
	opterr = 0;

	/* options */
	const char *short_opts = "hw:p:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "which", required_argument, NULL, 'w'},
		{ "pid", required_argument, NULL, 'p'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'w': /* Which */
			which = optarg;
			break;
		case 'p': /* PID */
			sscanf(optarg, "%d", &who);
			break;
		default:
			abort();
		}
	}

	if (hflags || !which) {
		usage(argv[0]);
		return 0;
	}

	/* parse which argument */
	if (strstr(which, "PRIO_PROCESS"))
		owhich = PRIO_PROCESS;
	else if (strstr(which, "PRIO_PGRP"))
		owhich = PRIO_PGRP;
	else if (strstr(which, "PRIO_USER"))
		owhich = PRIO_USER;

	/*
	 * sys_getpriority
	 *
	 *    SYSCALL_DEFINE2(getpriority,
	 *                    int, which,
	 *                    int, who)
	 */
	priority = syscall(__NR_getpriority, owhich, who);
	printf("The Process[%d] priority: %#lx\n", who, priority);
	
	return 0;
}
