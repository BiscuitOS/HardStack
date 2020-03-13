/*
 * sys_brk in C
 *
 * (C) 2020.03.13 BuddyZhang1 <buddy.zhang@aliyun.com>
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
/* __NR_brk */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_brk helper\n");
	printf("Usage:\n");
	printf("      %s <-b address>\n", program_name);
	printf("\n");
	printf("\t-b\t--boundary\tThe new boundary address for heap\n");
	printf("\ne.g:\n");
	printf("%s -b 0x60080000\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *address = NULL;
	unsigned long boundary_address;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hb:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "boundary", required_argument, NULL, 'b'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'b': /* Boundary address */
			address = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !address) {
		usage(argv[0]);
		return 0;
	}

	/* parse boundary address */
	sscanf(address, "%lx", &boundary_address);

	/*
	 * sys_brk
	 *
	 *    SYSCALL_DEFINE1(brk,
	 *                    unsigned long, brk)
	 */
	syscall(__NR_brk, boundary_address);

	return 0;
}
