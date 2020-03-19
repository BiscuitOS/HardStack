/*
 * sys_setitimer in C
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
/* __NR_getitimer/__NR_setitimer */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
#include <sys/time.h>

/* Architecture defined */
#ifndef __NR_getitimer
#define __NR_getitimer	105
#endif
#ifndef __NR_setitimer
#define __NR_setitimer	104
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_setitimer helper\n");
	printf("Usage:\n");
	printf("      %s <-w which> <-s v_sec> <-u v_usec> "
			"<-S i_sec> <-U i_usec>\n", program_name);
	printf("\n");
	printf("\t-w\t--whitch\tWhich timer.\n");
	printf("\t\t\tITIMER_REAL       Timers run in real time.\n");
	printf("\t\t\tITIMER_VIRTUAL    Timers run only when the process is executing\n");
	printf("\t\t\tITIMER_PROF       Timers run when the process is executing\n");
	printf("\t\t\t                  and when the system is executing on\n");
	printf("\t\t\t                  behalf of the process.\n");
	printf("\t-s\t--v_sec\tThe second for it_value.\n");
	printf("\t-u\t--v_usec\tThe usecond for it_value.\n");
	printf("\t-S\t--i_sec\tThe second for it_intervalue.\n");
	printf("\t-U\t--v_usec\tThe usecond for it_intervalue.\n");
	printf("\ne.g:\n");
	printf("%s -w ITIMER_REAL -s 10 -u 0 -S 10 -u 0\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *iwhich = NULL;
	int which = 0;
	int c, hflags = 0;
	struct itimerval tick, gtick;
	opterr = 0;

	/* options */
	const char *short_opts = "hw:s:u:S:U:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "which", required_argument, NULL, 'w'},
		{ "v_sec", required_argument, NULL, 's'},
		{ "v_usec", required_argument, NULL, 'u'},
		{ "i_sec", required_argument, NULL, 'S'},
		{ "u_usec", required_argument, NULL, 'U'},
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
		case 's': /* it_value.tv_sec */
			sscanf(optarg, "%ld", &tick.it_value.tv_sec);
			break;
		case 'u': /* it_value.tv_usec */
			sscanf(optarg, "%ld", &tick.it_value.tv_usec);
			break;
		case 'S': /* it_interval.tv_sec */
			sscanf(optarg, "%ld", &tick.it_interval.tv_sec);
			break;
		case 'U': /* it_interval.tv_usec */
			sscanf(optarg, "%ld", &tick.it_interval.tv_usec);
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
	 * sys_setitimer
	 *
	 *    SYSCALL_DEFINE3(setitimer,
	 *                    int, which,
	 *                    struct itimerval __user *, value,
	 *                    struct itimerval __user *, ovalue)
	 */
	syscall(__NR_setitimer, which, &tick, NULL);

	/*
	 * sys_getitimer
	 *
	 *    SYSCALL_DEFINE2(getitimer,
	 *                    int, which,
	 *                    struct itimerval __user *, value)
	 */
	syscall(__NR_getitimer, which, &gtick);
	printf("Timer:\n  sec: %ld\n  usec: %ld\n",
			gtick.it_value.tv_sec, gtick.it_value.tv_usec);
	printf("  i_sec: %ld\n  i_usec: %ld\n",
			gtick.it_interval.tv_sec, gtick.it_interval.tv_usec);
	return 0;
}
