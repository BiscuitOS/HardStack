/*
 * sys_getpgid in C
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
/* __NR_setpgid/__NR_getpgid */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_setpgid
#define __NR_setpgid	57
#endif
#ifndef __NR_getpgid
#define __NR_getpgid	132
#endif
#ifndef __NR_getpid
#define __NR_getpid	20
#endif

typedef int pid_t;

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_getpgid helper\n");
	printf("Usage:\n");
	printf("      %s <-g gid>\n", program_name);
	printf("\n");
	printf("\t-g\t--gid\tThe GID for setting.\n");
	printf("\ne.g:\n");
	printf("%s -g 0x86\n\n", program_name);
}

int main(int argc, char *argv[])
{
	pid_t gid = 0, pid, current_pgid;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hg:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "gid", required_argument, NULL, 'g'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'g': /* GID */
			sscanf(optarg, "%d", &gid);
			break;
		default:
			abort();
		}
	}

	if (hflags || !gid) {
		usage(argv[0]);
		return 0;
	}
	
	/*
	 * sys_getpid
	 *
	 *    SYSCALL_DEFINE0(getpid)
	 */
	pid = syscall(__NR_getpid);

	/*
	 * sys_setpgid
	 *
	 *    SYSCALL_DEFINE2(setpgid,
	 *                    pid_t, pid,
	 *                    pid_t, pgid)
	 */
	syscall(__NR_setpgid, pid, gid);

	/*
	 * sys_getpgid
	 *
	 *    SYSCALL_DEFINE1(getpgid,
	 *                    pid_t, pid)
	 */
	current_pgid = syscall(__NR_getpgid, pid);
	printf("PID %d's PGID: %d\n", pid, current_pgid);
	return 0;
}
