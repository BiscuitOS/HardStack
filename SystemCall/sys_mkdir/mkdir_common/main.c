/*
 * sys_mkdir in C
 *
 * (C) 2020.03.10 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_mkdir helper\n");
	printf("Usage:\n");
	printf("      %s <-p pathname> <-m mode>\n", program_name);
	printf("\n");
	printf("\t-p\t--path\tThe full path for creating dentory.\n");
	printf("\t-m\t--mode\tThe mode for creating dentory.\n\n");
}

int main(int argc, char *argv[])
{
	char *path = NULL;
	char *mode = NULL;
	int mode_value;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hp:m:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "path", required_argument, NULL, 'p'},
		{ "mode", required_argument, NULL, 'm'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'p': /* Path */
			path = optarg;
			break;
		case 'm': /* mode */
			mode = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !path || !mode)
		usage(argv[0]);

	sscanf(mode, "%o", &mode_value);

	/* mkdir on current dirent */
	mkdir(path, mode_value);

	return 0;
}
