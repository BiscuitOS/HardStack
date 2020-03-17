/*
 * sys_times in C
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
/* __NR_times */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
#include <sys/times.h>

/* Architecture defined */
#ifndef __NR_times
#define __NR_times	5
#endif
#define CLOCKS_PER_SEC	(100)

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_times helper\n");
	printf("Usage:\n");
	printf("      %s\n", program_name);
	printf("\n");
	printf("\ne.g:\n");
	printf("%s\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	opterr = 0;
	struct tms time;
	clock_t begin, end;

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
	 * sys_times
	 *
	 *    SYSCALL_DEFINE1(times,
	 *                    struct tms __user *, tbuf)
	 */
	begin = syscall(__NR_times, &time);

	printf("Calculate running time.\n");
	sleep(1);

	end = syscall(__NR_times, &time);

	printf("Cost %lf sec\n", (double)(end - begin) / CLOCKS_PER_SEC);
	
	return 0;
}
