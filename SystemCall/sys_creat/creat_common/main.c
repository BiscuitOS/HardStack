/*
 * sys_creat in C
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
/* creat */
#include <fcntl.h>
/* __NR_creat/__NR_close */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_creat
#define __NR_creat	8
#endif
#ifndef __NR_close
#define __NR_close	6
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_creat helper\n");
	printf("Usage:\n");
	printf("      %s <-p pathname> <-m mode>\n", program_name);
	printf("\n");
	printf("\t-p\t--path\tThe full path for opening.\n");
	printf("\t-m\t--mode\tThe mode for opening.\n");
	printf("\t\t\tS_IRUSR\n");
	printf("\t\t\tS_IWUSR\n");
	printf("\t\t\tS_IXUSR\n");
	printf("\t\t\tS_IRWXU\n");
	printf("\t\t\tS_IRGRP\n");
	printf("\t\t\tS_IWGRP\n");
	printf("\t\t\tS_IXGRP\n");
	printf("\t\t\tS_IRWXG\n");
	printf("\t\t\tS_IROTH\n");
	printf("\t\t\tS_IWOTH\n");
	printf("\t\t\tS_IXOTH\n");
	printf("\t\t\tS_IRWXO\n");
	printf("\ne.g:\n");
	printf("%s -p BiscuitOS_file -m S_IRUSR,S_IRGRP\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *path = NULL;
	char *mode = NULL;
	mode_t omode = 0;
	int c, hflags = 0;
	int fd;
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

	if (hflags || !path || !mode) {
		usage(argv[0]);
		return 0;
	}

	/* parse mode argument */
	if (mode) {
		if (strstr(mode, "S_IRUSR"))
			omode |= S_IRUSR;
		if (strstr(mode, "S_IWUSR"))
			omode |= S_IWUSR;
		if (strstr(mode, "S_IXUSR"))
			omode |= S_IXUSR;
		if (strstr(mode, "S_IRWXU"))
			omode |= S_IRWXU;
		if (strstr(mode, "S_IRGRP"))
			omode |= S_IRGRP;
		if (strstr(mode, "S_IWGRP"))
			omode |= S_IWGRP;
		if (strstr(mode, "S_IXGRP"))
			omode |= S_IXGRP;
		if (strstr(mode, "S_IRWXG"))
			omode |= S_IRWXG;
		if (strstr(mode, "S_IROTH"))
			omode |= S_IROTH;
		if (strstr(mode, "S_IWOTH"))
			omode |= S_IWOTH;
		if (strstr(mode, "S_IXOTH"))
			omode |= S_IXOTH;
		if (strstr(mode, "S_IRWXO"))
			omode |= S_IRWXO;
	}

	/*
	 * sys_creat
	 *
	 *    SYSCALL_DEFINE2(creat,
	 *                    const char __user *, pathname,
	 *                    umode_t, mode)
	 */
	fd = syscall(__NR_creat, path, omode);

	/*
	 * sys_close()
	 *
	 *    SYSCALL_DEFINE1(close,
	 *                    unsigned int, fd)
	 *
	 */
	syscall(__NR_close, (unsigned int)fd);
	return 0;
}
