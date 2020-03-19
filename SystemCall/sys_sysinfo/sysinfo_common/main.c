/*
 * sys_sysinfo in C
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
/* __NR_sysinfo */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
#include <sys/sysinfo.h>

/* Architecture defined */
#ifndef __NR_sysinfo
#define __NR_sysinfo	116
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_sysinfo helper\n");
	printf("Usage:\n");
	printf("      %s\n", program_name);
	printf("\ne.g:\n");
	printf("%s\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	struct sysinfo info;
	opterr = 0;

	/* options */
	const char *short_opts = "h";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
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
	 * sys_sysinfo
	 *
	 *    SYSCALL_DEFINE1(sysinfo,
	 *                    struct sysinfo __user *, info)
	 */
	syscall(__NR_sysinfo, &info);

	printf("uptime:                   %ld\n", info.uptime);
	printf("1 min load average:       %lu\n", info.loads[0]);
	printf("5 min load average:       %lu\n", info.loads[1]);
	printf("15 min load average:      %lu\n", info.loads[2]);
	printf("totalram:                 %lu\n", info.totalram);
	printf("freeram:                  %lu\n", info.freeram);
	printf("procs:                    %u\n",  info.procs);

	return 0;
}
