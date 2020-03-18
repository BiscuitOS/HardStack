/*
 * sys_symlink in C
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
/* __NR_symlink */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_symlink
#define __NR_symlink	83
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_symlink helper\n");
	printf("Usage:\n");
	printf("      %s <-o orignal> <-n new>\n", program_name);
	printf("\n");
	printf("\t-o\t--old\tThe original file name.\n");
	printf("\t-n\t--new\tThe new symbol name.\n");
	printf("\ne.g:\n");
	printf("%s -o BiscuitOS_file -n BiscuitOS_sym \n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *old = NULL;
	char *new = NULL;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "ho:n:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "old", required_argument, NULL, 'o'},
		{ "new", required_argument, NULL, 'n'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'o': /* Old filename */
			old = optarg;
			break;
		case 'n': /* new symbol filename */
			new = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !old || !new) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_symlink
	 *
	 *    SYSCALL_DEFINE2(symlink,
	 *                    const char __user *, oldname,
	 *                    const char __user *, newname)
	 */
	syscall(__NR_symlink, old, new);
	return 0;
}
