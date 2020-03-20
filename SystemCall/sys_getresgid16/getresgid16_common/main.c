/*
 * sys_getresgid16 in C
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
/* __NR_setresgid16/__NR_getresgid16 */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_setresgid16
#define __NR_setresgid16	170
#endif
#ifndef __NR_getresgid16
#define __NR_getresgid16	171
#endif

typedef unsigned int old_gid_t;

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_getresgid16 helper\n");
	printf("Usage:\n");
	printf("      %s\n", program_name);
	printf("\ne.g:\n");
	printf("%s\n\n", program_name);
}

int main(int argc, char *argv[])
{
	old_gid_t rgid, egid, sgid;
	int c, hflags = 0;
	opterr = 0;

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
	 * sys_getresgid16
	 *
	 *    SYSCALL_DEFINE3(getresgid16,
	 *                    old_gid_t __user *, rgidp,
	 *                    old_gid_t __user *, egidp,
	 *                    old_gid_t __user *, sgidp)
	 */
	syscall(__NR_getresgid16, &rgid, &egid, &sgid);
	printf("RGID: %d\nEGID: %d\nSGID: %d\n", rgid, egid, sgid);

	return 0;
}
