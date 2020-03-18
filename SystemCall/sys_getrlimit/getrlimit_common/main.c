/*
 * sys_getrlimit in C
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
/* __NR_setrlimit/__NR_getrlimit */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
/* rlimit flags */
#include <sys/resource.h>

/* Architecture defined */
#ifndef __NR_setrlimit
#define __NR_setrlimit	75
#endif
#ifndef __NR_getrlimit
#define __NR_getrlimit	191
#endif

/* Architecture flags */
#ifndef RLIMIT_CPU
#define RLIMIT_CPU		0	/* CPU time in sec */ 
#endif
#ifndef RLIMIT_FSIZE
#define RLIMIT_FSIZE		1	/* Maximum filesize */
#endif
#ifndef RLIMIT_DATA
#define RLIMIT_DATA		2	/* max data size */
#endif
#ifndef RLIMIT_STACK
#define RLIMIT_STACK		3	/* max stack size */
#endif
#ifndef RLIMIT_CORE
#define RLIMIT_CORE		4	/* max core file size */
#endif
#ifndef RLIMIT_RSS
#define RLIMIT_RSS		5	/* max resident set size */
#endif
#ifndef RLIMIT_NPROC
#define RLIMIT_NPROC		6	/* max number of processes */
#endif
#ifndef RLIMIT_NOFILE
#define RLIMIT_NOFILE		7	/* max number of open files */
#endif
#ifndef RLIMIT_MEMLOCK
#define RLIMIT_MEMLOCK		8	/* max locked-in-memory address space */
#endif
#ifndef RLIMIT_AS
#define RLIMIT_AS		9	/* address space limit */
#endif
#ifndef RLIMIT_LOCKS
#define RLIMIT_LOCKS		10	/* maximum file locks held */
#endif
#ifndef RLIMIT_SIGPENDING 
#define RLIMIT_SIGPENDING	11	/* max number of pending signals */
#endif
#ifndef RLIMIT_MSGQUEUE 
#define RLIMIT_MSGQUEUE		12	/* maximum bytes in POSIX mqueues */
#endif
#ifndef RLIMIT_NICE
#define RLIMIT_NICE		13	/* max nice prio allowed to raise to
					   0-39 for nice level 19 .. -20 */
#endif
#ifndef RLIMIT_RTPRIO 
#define RLIMIT_RTPRIO		14	/* maximum realtime priority */
#endif
#ifndef RLIMIT_RTTIME
#define RLIMIT_RTTIME		15	/* timeout for RT tasks in us */
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_getrlimit helper\n");
	printf("Usage:\n");
	printf("      %s <-l resource_item>\n", program_name);
	printf("\n");
	printf("\t-l\t--res\tThe resource iterm for limit.\n");
	printf("\t\t\tRLIMIT_CPU         CPU time in sec\n");
	printf("\t\t\tRLIMIT_FSIZE       Maximum filesize\n");
	printf("\t\t\tRLIMIT_DATA        max data size\n");
	printf("\t\t\tRLIMIT_STACK       max stack size\n");
	printf("\t\t\tRLIMIT_CORE        max core file size\n");
	printf("\t\t\tRLIMIT_RSS         max resident set size\n");
	printf("\t\t\tRLIMIT_NPROC       max number of processes\n");
	printf("\t\t\tRLIMIT_NOFILE      max number of open files\n");
	printf("\t\t\tRLIMIT_MEMLOCK     max locked-in-memory address space\n");
	printf("\t\t\tRLIMIT_AS          address space limit\n");
	printf("\t\t\tRLIMIT_LOCKS       maximum file locks held\n");
	printf("\t\t\tRLIMIT_SIGPENDING  max number of pending signals\n");
	printf("\t\t\tRLIMIT_MSGQUEUE    maximum bytes in POSIX mqueues\n");
	printf("\t\t\tRLIMIT_NICE        max nice prio allowed to raise to\n");
	printf("\t\t\t                   0-39 for nice level 19 .. -20\n");
	printf("\t\t\tRLIMIT_RTPRIO      maximum realtime priority\n");
	printf("\t\t\tRLIMIT_RTTIME      timeout for RT tasks in us\n");
	printf("\t\t\t\n");
	printf("\ne.g:\n");
	printf("%s -l RLIMIT_FSIZE\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *res = NULL;
	int c, hflags = 0;
	struct rlimit glimit;
	unsigned int item;

	opterr = 0;

	/* options */
	const char *short_opts = "hl:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "res", required_argument, NULL, 'l'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'l': /* limit resource */
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

	/* parse resource item */
	if (strstr(res, "RLIMIT_CPU"))
		item = RLIMIT_CPU;
	else if (strstr(res, "RLIMIT_FSIZE"))
		item = RLIMIT_FSIZE;
	else if (strstr(res, "RLIMIT_DATA"))
		item = RLIMIT_DATA;
	else if (strstr(res, "RLIMIT_STACK"))
		item = RLIMIT_STACK;
	else if (strstr(res, "RLIMIT_CORE"))
		item = RLIMIT_CORE;
	else if (strstr(res, "RLIMIT_RSS"))
		item = RLIMIT_RSS;
	else if (strstr(res, "RLIMIT_NPROC"))
		item = RLIMIT_NPROC;
	else if (strstr(res, "RLIMIT_NOFILE"))
		item = RLIMIT_NOFILE;
	else if (strstr(res, "RLIMIT_MEMLOCK"))
		item = RLIMIT_MEMLOCK;
	else if (strstr(res, "RLIMIT_AS"))
		item = RLIMIT_AS;
	else if (strstr(res, "RLIMIT_LOCKS"))
		item = RLIMIT_LOCKS;
	else if (strstr(res, "RLIMIT_SIGPENDING"))
		item = RLIMIT_SIGPENDING;
	else if (strstr(res, "RLIMIT_MSGQUEUE"))
		item = RLIMIT_MSGQUEUE;
	else if (strstr(res, "RLIMIT_NICE"))
		item = RLIMIT_NICE;
	else if (strstr(res, "RLIMIT_RTPRIO"))
		item = RLIMIT_RTPRIO;
	else if (strstr(res, "RLIMIT_RTTIME"))
		item = RLIMIT_RTTIME;

	/*
	 * sys_getrlimit
	 *
	 *    SYSCALL_DEFINE2(getrlimit,
	 *                    unsigned int, resource,
	 *                    struct rlimit __user *, rlim) 
	 */
	syscall(__NR_getrlimit, item, &glimit);
	printf("%s:\n  Current limit: %ld\n  Max limit: %ld\n",
			res, glimit.rlim_cur, glimit.rlim_max);

	return 0;
}
