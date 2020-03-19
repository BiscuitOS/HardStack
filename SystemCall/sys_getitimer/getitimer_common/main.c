/*
 * sys_getitimer in C
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
/* __NR_getitimer */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
#include <sys/time.h>

/* Architecture defined */
#ifndef __NR_getitimer
#define __NR_getitimer	105
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_getitimer helper\n");
	printf("Usage:\n");
	printf("      %s <-w which>\n", program_name);
	printf("\n");
	printf("\t-w\t--whitch\tWhich timer.\n");
	printf("\t\t\tITIMER_REAL       Timers run in real time.\n");
	printf("\t\t\tITIMER_VIRTUAL    Timers run only when the process is executing\n");
	printf("\t\t\tITIMER_PROF       Timers run when the process is executing\n");
	printf("\t\t\t                  and when the system is executing on\n");
	printf("\t\t\t                  behalf of the process.\n");
	printf("\t\t\t\n");
	printf("\ne.g:\n");
	printf("%s -p ITIMER_REAL\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *iwhich = NULL;
	int which = 0;
	int c, hflags = 0;
	struct itimerval tick;
	opterr = 0;

	/* options */
	const char *short_opts = "hw:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "which", required_argument, NULL, 'w'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'w': /* which */
			iwhich = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !iwhich) {
		usage(argv[0]);
		return 0;
	}

	/* parse which argument */
	if (strstr(iwhich, "ITIMER_REAL"))
		which = ITIMER_REAL;
	else if (strstr(iwhich, "ITIMER_VIRTUAL"))
		which = ITIMER_VIRTUAL;
	else if (strstr(iwhich, "ITIMER_PROF"))
		which = ITIMER_PROF;

	/*
	 * sys_getitimer
	 *
	 *    SYSCALL_DEFINE2(getitimer,
	 *                    int, which,
	 *                    struct itimerval __user *, value)
	 */
	syscall(__NR_getitimer, which, &tick);
	printf("Timer:\n  sec: %ld\n  usec: %ld\n",
			tick.it_value.tv_sec, tick.it_value.tv_usec);
	return 0;
}
