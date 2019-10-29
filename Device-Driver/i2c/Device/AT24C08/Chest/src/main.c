/*
 * Chest
 *
 * (C) 2019.10.28 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "command.h"
#include "chest.h"

/* Useage Interface */
void usage(const char *program_name)
{
	int idx;

	printf("%s 1.1.2(build %s)\n", program_name, __DATE__);
	for (idx = 0; cmd_lists[idx].cmdline; idx++) {
		if (cmd_lists[idx].flags & CHEST_SECT)
			continue;
		printf("[Describe] %s\n    ", cmd_lists[idx].desc);
		printf(cmd_lists[idx].usage, program_name);
		printf("\n");
	}
}

int main(int argc,char *argv[])
{
	const char *short_opts = "hr:w:";
	const struct option long_opts[] = {
		{"help", no_argument, NULL, 'h'},
		{"Read", required_argument, NULL, 'r'},
		{"Write", required_argument, NULL, 'w'},
		{0, 0, 0, 0}
	};
	int c;
	opterr = 0;

	/* chest initialize */
	chest_probe();

	while((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch(c) {
		case 'h':
			usage(argv[0]);
			break;
		case 'r':
			chest_read_interface(optarg, argv);
			break;
		case 'w':
			chest_write_interface(optarg, argv, optind);
			break;
		case '?':
			exit(1);
		default:
			abort();
		}
	}

	/* chest release */
	chest_remove();

	return 0;
}
