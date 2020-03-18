/*
 * sys_sethostname in C
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
/* __NR_sethostname */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_sethostname
#define __NR_sethostname	74
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_sethostname helper\n");
	printf("Usage:\n");
	printf("      %s <-n hostname>\n", program_name);
	printf("\n");
	printf("\t-n\t--hostname\tThe new hostname\n");
	printf("\ne.g:\n");
	printf("%s -n BiscuitOS_Host\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *hostname = NULL;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hn:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "hostname", required_argument, NULL, 'n'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'n': /* Hostname */
			hostname = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !hostname) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_sethostname
	 *
	 *    SYSCALL_DEFINE2(sethostname,
	 *                    char __user *, name,
	 *                    int, len)
	 */
	syscall(__NR_sethostname, hostname, strlen(hostname));
	return 0;
}
