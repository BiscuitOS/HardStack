/*
 * sys_getsid in C
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
/* __NR_getsid */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_getsid
#define __NR_getsid	147
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_getsid helper\n");
	printf("Usage:\n");
	printf("      %s <-p pid>\n", program_name);
	printf("\t-p\t--pid\tThe process PID.\n");
	printf("\ne.g:\n");
	printf("%s -p 876\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	pid_t pid, sid;
	opterr = 0;

	/* options */
	const char *short_opts = "hp:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "pid", required_argument, NULL, 'p'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'p':
			sscanf(optarg, "%d", &pid);
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
	 * sys_getsid
	 *
	 *    SYSCALL_DEFINE1(getsid,
	 *                    pid_t, pid)
	 */
	sid = syscall(__NR_getsid, pid);
	printf("PID %d's SID: %d\n", pid, sid);

	return 0;
}
