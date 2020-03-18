/*
 * sys_dup2 in C
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
/* open */
#include <fcntl.h>
/* __NR_open/__NR_dup */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_open
#define __NR_open	5
#endif
#ifndef __NR_close
#define __NR_close	6
#endif
#ifndef __NR_dup2
#define __NR_dup2	63
#endif

/* Architecture flags */
#ifndef O_TMPFILE
#define O_TMPFILE		020000000
#endif
#ifndef O_DIRECT
#define O_DIRECT		00040000	/* direct disk access hint */
#endif
#ifndef O_PATH
#define O_PATH			010000000
#endif
#ifndef O_NATIME
#define O_NATIME		01000000
#endif
#ifndef O_LARGEFILE
#define O_LARGEFILE		00100000
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_dup2 helper\n");
	printf("Usage:\n");
	printf("      %s <-p pathname> <-f flags> <-m mode>\n", program_name);
	printf("\n");
	printf("\t-p\t--path\tThe full path for opening.\n");
	printf("\t-f\t--flags\tThe flags for opening.\n");
	printf("\t\t\tO_ACCMODE\n");
	printf("\t\t\tO_RDONLY\n");
	printf("\t\t\tO_WRONLY\n");
	printf("\t\t\tO_RDWR\n");
	printf("\t\t\tO_CLOEXEC\n");
	printf("\t\t\tO_DIRECTORY\n");
	printf("\t\t\tO_NOFOLLOW\n");
	printf("\t\t\tO_CREAT\n");
	printf("\t\t\tO_EXCL\n");
	printf("\t\t\tO_NOCTTY\n");
	printf("\t\t\tO_TMPFILE\n");
	printf("\t\t\tO_TRUNC\n");
	printf("\t\t\tO_APPEND\n");
	printf("\t\t\tO_ASYNC\n");
	printf("\t\t\tO_DIRECT\n");
	printf("\t\t\tO_DSYNC\n");
	printf("\t\t\tO_LARGEFILE\n");
	printf("\t\t\tO_NATIME\n");
	printf("\t\t\tO_NONBLOCK\n");
	printf("\t\t\tO_SYNC\n");
	printf("\t\t\tO_PATH\n");
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
	printf("%s -p BiscuitOS_file -f O_RDWR,O_CREAT "
			"-m S_IRUSR,S_IRGRP\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *path = NULL;
	char *mode = NULL;
	char *flags = NULL;
	mode_t omode = 0;
	int mode_value;
	int c, hflags = 0;
	int oflags = 0;
	int fd, fd_dup, fd1;
	opterr = 0;

	/* options */
	const char *short_opts = "hp:f:m:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "path", required_argument, NULL, 'p'},
		{ "flags", required_argument, NULL, 'f'},
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
		case 'f': /* flags */
			flags = optarg;
			break;
		case 'm': /* mode */
			mode = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !path || !flags || !mode) {
		usage(argv[0]);
		return 0;
	}

	/* parse flags argument */
	if (strstr(flags, "O_ACCMODE"))
		oflags |= O_ACCMODE;
	if (strstr(flags, "O_RDONLY"))
		oflags |= O_RDONLY;
	if (strstr(flags, "O_WRONLY"))
		oflags |= O_WRONLY;
	if (strstr(flags, "O_RDWR"))
		oflags |= O_RDWR;
	if (strstr(flags, "O_CLOEXEC"))
		oflags |= O_CLOEXEC;
	if (strstr(flags, "O_DIRECTORY"))
		oflags |= O_DIRECTORY;
	if (strstr(flags, "O_NOFOLLOW"))
		oflags |= O_NOFOLLOW;
	if (strstr(flags, "O_CREAT"))
		oflags |= O_CREAT;
	if (strstr(flags, "O_EXCL"))
		oflags |= O_EXCL;
	if (strstr(flags, "O_NOCTTY"))
		oflags |= O_NOCTTY;
	if (strstr(flags, "O_TMPFILE"))
		oflags |= O_TMPFILE;
	if (strstr(flags, "O_TRUNC"))
		oflags |= O_TRUNC;
	if (strstr(flags, "O_APPEND"))
		oflags |= O_APPEND;
	if (strstr(flags, "O_ASYNC"))
		oflags |= O_ASYNC;
	if (strstr(flags, "O_DIRECT"))
		oflags |= O_DIRECT;
	if (strstr(flags, "O_DSYNC"))
		oflags |= O_DSYNC;
	if (strstr(flags, "O_LARGEFILE"))
		oflags |= O_LARGEFILE;
	if (strstr(flags, "O_NATIME"))
		oflags |= O_NATIME;
	if (strstr(flags, "O_NONBLOCK"))
		oflags |= O_NONBLOCK;
	if (strstr(flags, "O_SYNC"))
		oflags |= O_SYNC;
	if (strstr(flags, "O_PATH"))
		oflags |= O_PATH;


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
	 * sys_open() 
	 *
	 *    SYSCALL_DEFINE3(open, 
	 *                    const char __user *, filename, 
	 *                    int, flags,
	 *                    umode_t, mode)
	 */
	if (mode) {
		fd  = syscall(__NR_open, path, oflags, omode);
		fd1 = syscall(__NR_open, "BiscuitOS_file", oflags, omode);
	} else {
		fd  = syscall(__NR_open, path, oflags);
		fd1 = syscall(__NR_open, "BiscuitOS_file", oflags);
	}
	if (fd < 0) {
		printf("Open: Can't open %s err %d\n", path, fd);
		return -1;
	}
	if (fd1 < 0) {
		printf("Open: Can't open BiscuitOS_file err %d\n", fd1);
		/*
		 * sys_close
		 *
		 *    SYSCALL_DEFINE1(close,
		 *                    unsigned int, fd)
		 */
		syscall(__NR_close, fd);
		return -1;
	}

	/*
	 * sys_dup2
	 *
	 *    SYSCALL_DEFINE2(dup2,
	 *                    unsigned int, oldfd,
	 *                    unsigned int, newfd)
	 */
	fd_dup = syscall(__NR_dup2, fd, fd1);
	printf("Original fd: %d Dup-fd: %d OLD-fd: %d\n", fd, fd_dup, fd1);

	/*
	 * sys_close()
	 *
	 *    SYSCALL_DEFINE1(close,
	 *                    unsigned int, fd)
	 *
	 */
	syscall(__NR_close, (unsigned int)fd1);
	syscall(__NR_close, (unsigned int)fd);
	syscall(__NR_close, (unsigned int)fd_dup);
	return 0;
}
