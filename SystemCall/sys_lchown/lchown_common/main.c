/*
 * sys_lchown in C
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
/* __NR_lchown */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_lchown
#define __NR_lchown	198
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_lchown helper\n");
	printf("Usage:\n");
	printf("      %s <-p filename> <-u uid> <-g gid>\n", program_name);
	printf("\n");
	printf("\t-p\t--path\tThe path for file.\n");
	printf("\t-u\t--uid\tThe UID for changing.\n");
	printf("\t-g\t--gid\tThe GID for changing.\n");
	printf("\ne.g:\n");
	printf("%s -p BiscuitOS_file -u 89 -g 876\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	char *path = NULL;
	gid_t gid;
	uid_t uid;
	opterr = 0;

	/* options */
	const char *short_opts = "hp:g:u:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "path", required_argument, NULL, 'p'},
		{ "uid", required_argument, NULL, 'u'},
		{ "gid", required_argument, NULL, 'g'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'p': /* path */
			path = optarg;
			break;
		case 'u': /* UID */
			sscanf(optarg, "%d", &uid);
			break;
		case 'g': /* GID */
			sscanf(optarg, "%d", &gid);
			break;
		default:
			abort();
		}
	}

	if (hflags || !path) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_lchown
	 *
	 *    SYSCALL_DEFINE3(lchown,
	 *                    const char __user *, filename,
	 *                    uid_t, user,
	 *                    gid_t, group)
	 */
	syscall(__NR_lchown, path, uid, gid);
	return 0;
}
