/*
 * sys_exit in C
 *
 * (C) 2020.03.15 BuddyZhang1 <buddy.zhang@aliyun.com>
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
/* __NR_exit */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_exit
#define __NR_exit	1
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_exit helper\n");
	printf("Usage:\n");
	printf("      %s <-n error_no>\n", program_name);
	printf("\n");
	printf("\t-n\t--error_no\tThe exit code.\n");
	printf("\ne.g:\n");
	printf("%s -n 0\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *code = NULL;
	int exit_code;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hn:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "error_no", required_argument, NULL, 'n'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'n': /* Error code */
			code = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !code) {
		usage(argv[0]);
		return 0;
	}

	/* parse error_no */
	sscanf(code, "%d", &exit_code);

	/*
	 * sys_exit
	 *
	 *    SYSCALL_DEFINE1(exit,
	 *                    int, error_code)
	 */
	syscall(__NR_exit, exit_code);
	return 0;
}
