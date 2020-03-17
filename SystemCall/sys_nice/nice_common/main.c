/*
 * sys_nice in C
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
/* __NR_nice */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_nice
#define __NR_nice	34
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_nice helper\n");
	printf("Usage:\n");
	printf("      %s <-n nice>\n", program_name);
	printf("\n");
	printf("\t-n\t--nice\tThe current process running level\n");
	printf("\ne.g:\n");
	printf("%s -n 10\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int nice = 0;
	int c, hflags = 0;
	opterr = 0;
	int current_nice = 0;

	/* options */
	const char *short_opts = "hn:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "nice", required_argument, NULL, 'n'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'n': /* nice */
			sscanf(optarg, "%d", &nice);
			break;
		default:
			abort();
		}
	}

	if (hflags || !nice) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_nice
	 *
	 *    SYSCALL_DEFINE1(nice, int, increment)
	 */
	current_nice = syscall(__NR_nice, nice);
	if (current_nice == -1)
		printf("Can't set priority as %d\n", nice);
	else
		printf("Succeed set priority as %d\n", current_nice);

	return 0;
}
