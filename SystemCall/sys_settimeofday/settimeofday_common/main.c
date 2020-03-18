/*
 * sys_settimeofday in C
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
/* __NR_gettimeofday/__NR_settimeofday */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
#include <sys/time.h>

/* Architecture defined */
#ifndef __NR_gettimeofday
#define __NR_gettimeofday	78
#endif
#ifndef __NR_settimeofday
#define __NR_settimeofday	79
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_settimeofday helper\n");
	printf("Usage:\n");
	printf("      %s <-s tv_sec> <-u tv_usec> "
			"<-m tz_min> <-d tv_dst>\n", program_name);
	printf("\t-s\t--tv_sec\tThe secound for tv.\n");
	printf("\t-u\t--tv_usec\tThe Mic-secound for tv.\n");
	printf("\t-m\t--tz_min\tThe minute for tz.\n");
	printf("\t-d\t--tz_dst\tThe disten for tz.\n");
	printf("\ne.g:\n");
	printf("%s\n\n", program_name);
}

int main(int argc, char *argv[])
{
	int c, hflags = 0;
	struct timeval tv, gtv;
	struct timezone tz, gtz;
	opterr = 0;

	/* options */
	const char *short_opts = "hs:u:m:d:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "sec", required_argument, NULL, 's'},
		{ "usec", required_argument, NULL, 'u'},
		{ "minute", required_argument, NULL, 'm'},
		{ "disten", required_argument, NULL, 'd'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 's':
			sscanf(optarg, "%ld", &tv.tv_sec);
			break;
		case 'u':
			sscanf(optarg, "%ld", &tv.tv_usec);
			break;
		case 'm':
			sscanf(optarg, "%d", &tz.tz_minuteswest);
			break;
		case 'd':
			sscanf(optarg, "%d", &tz.tz_dsttime);
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
	 * sys_settimeofday
	 *
	 *    SYSCALL_DEFINE2(settimeofday,
	 *                    struct timeval __user *, tv,
	 *                    struct timezone __user *, tz)
	 */
	syscall(__NR_settimeofday, &tv, &tz);

	/*
	 * sys_gettimeofday
	 *
	 *    SYSCALL_DEFINE2(gettimeofday,
	 *                    struct timeval __user *, tv,
	 *                    struct timezone __user *, tz)
	 */
	syscall(__NR_gettimeofday, &gtv, &gtz);
	printf("TV_SEC:    %lld\n", (unsigned long long)gtv.tv_sec);
	printf("TV_USEC:   %lld\n", (unsigned long long)gtv.tv_usec);
	printf("TZ_MIN:    %d\n", gtz.tz_minuteswest);
	printf("TZ_DST:    %d\n", gtz.tz_dsttime);

	return 0;
}
