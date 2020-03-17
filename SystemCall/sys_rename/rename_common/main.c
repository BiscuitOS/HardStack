/*
 * sys_rename in C
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
/* __NR_rename */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_rename
#define __NR_rename	38
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_rename helper\n");
	printf("Usage:\n");
	printf("      %s <-o old_name> <-n new_name>\n", program_name);
	printf("\n");
	printf("\t-o\t--old_name\tThe original name.\n");
	printf("\t-n\t--new_name\tThe new name.\n");
	printf("\ne.g:\n");
	printf("%s -o bs0 -n bs1\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *oldname = NULL;
	char *newname = NULL;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "ho:n:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "old_name", required_argument, NULL, 'o'},
		{ "new_name", required_argument, NULL, 'n'},
		{ "mode", required_argument, NULL, 'm'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'o': /* Original name */
			oldname = optarg;
			break;
		case 'n': /* new name */
			newname = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !oldname || !newname) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_rename
	 *
	 *    SYSCALL_DEFINE2(rename,
	 *                    const char __user *, oldname,
	 *                    const char __user *, newname) 
	 */
	syscall(__NR_rename, oldname, newname);
	return 0;
}
