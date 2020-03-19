/*
 * sys_statfs in C
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
/* __NR_statfs */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
#include <sys/statfs.h>

/* Architecture defined */
#ifndef __NR_statfs
#define __NR_ststfs	99
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_statfs helper\n");
	printf("Usage:\n");
	printf("      %s <-p pathname>\n", program_name);
	printf("\n");
	printf("\t-p\t--path\tThe full path for searching.\n");
	printf("\ne.g:\n");
	printf("%s -p /tmp/\n\n", program_name);
}

int main(int argc, char *argv[])
{
	unsigned long long blocksize, totalsize;
	unsigned long long disks, freedisk;
	char *path = NULL;
	int c, hflags = 0;
	struct statfs info;
	opterr = 0;

	/* options */
	const char *short_opts = "hp:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "path", required_argument, NULL, 'p'},
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
		default:
			abort();
		}
	}

	if (hflags || !path) {
		usage(argv[0]);
		return 0;
	}

	/*
	 * sys_statfs
	 *
	 *    SYSCALL_DEFINE2(statfs,
	 *                    const char __user *, pathname,
	 *                    struct statfs __user *, buf)
	 */
	syscall(__NR_statfs, path, &info);

	blocksize = info.f_bsize;
	totalsize = blocksize * info.f_blocks;
	disks = info.f_bfree * blocksize;
	freedisk = info.f_bavail * blocksize;
	printf("Total size = %llu B = %llu KB = %llu MB = %llu GB\n",
		totalsize, totalsize >> 10, totalsize >> 20, totalsize >> 30);
	printf("Disk_Free = %llu MB = %llu GB\n",
		disks >> 20, disks >> 30);
	printf("Disk_available = %llu MB = %llu GB\n",
		freedisk >> 20, freedisk >> 30);

	return 0;
}
