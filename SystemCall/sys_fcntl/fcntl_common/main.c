/*
 * sys_fcntl in C
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
/* __NR_open */
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
#ifndef __NR_fcntl
#define __NR_fcntl	55
#endif

/* Architecture fcntl flags */
#ifndef F_SETSIG
#define F_SETSIG	10
#endif
#ifndef F_GETSIG
#define F_GETSIG	11
#endif
#ifndef F_SETOWN_EX
#define F_SETOWN_EX	15
#endif
#ifndef F_GETOWN_EX
#define F_GETOWN_EX	16
#endif
#ifndef F_GETOWNER_UIDS
#define F_GETOWNER_UIDS	17
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
	printf("BiscuitOS: sys_fcntl helper\n");
	printf("Usage:\n");
	printf("      %s <-p pathname> <-f flags> <-m mode> "
			"<-c cmd> <-a arg>\n", program_name);
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
	printf("\t-c\t--cmd\tThe command for fcntl.\n");
	printf("\t\t\tF_DUPFD        dup\n");
	printf("\t\t\tF_GETFD        get close_on_exec\n");
	printf("\t\t\tF_SETFD        set/clear close_on_exec\n");
	printf("\t\t\tF_GETFL        get file->f_flags\n");
	printf("\t\t\tF_SETFL        set file->f_flags\n");
	printf("\t\t\tF_GETLK        get special lock information\n");
	printf("\t\t\tF_SETLK        set special lock\n");
	printf("\t\t\tF_SETLKW       set special lock\n");
	printf("\t\t\tF_SETOWN\n");
	printf("\t\t\tF_GETOWN\n");
	printf("\t\t\tF_SETSIG\n");
	printf("\t\t\tF_GETSIG\n");
	printf("\t\t\tF_SETOWN_EX\n");
	printf("\t\t\tF_GETOWN_EX\n");
	printf("\t\t\tF_GETOWNER_UIDS\n");
	printf("\t\t\tF_OFD_GETLK\n");
	printf("\t\t\tF_OFD_SETLK\n");
	printf("\t\t\tF_OFD_SETLKW\n");
	printf("\t-a\t--arg\tThe argument for command.\n");
	printf("\ne.g:\n");
	printf("%s -p BiscuitOS_file -f O_RDWR,O_CREAT "
			"-m S_IRUSR,S_IRGRP -c F_GETFL -a 0\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *path = NULL;
	char *mode = NULL;
	char *flags = NULL;
	char *cmd = NULL;
	mode_t omode = 0;
	int mode_value;
	int c, hflags = 0;
	int oflags = 0;
	int fd;
	unsigned int ocmd = 0;
	unsigned long args = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hp:f:m:c:a:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "path", required_argument, NULL, 'p'},
		{ "flags", required_argument, NULL, 'f'},
		{ "mode", required_argument, NULL, 'm'},
		{ "command", required_argument, NULL, 'c'},
		{ "argument", required_argument, NULL, 'a'},
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
		case 'c': /* command */
			cmd = optarg;
			break;
		case 'a': /* mode */
			sscanf(optarg, "%ld", &args);
			break;
		default:
			abort();
		}
	}

	if (hflags || !path || !flags || !mode || !cmd) {
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

	/* parse command argument */
	if (strstr(cmd, "F_DUPFD"))
		ocmd = F_DUPFD;
	else if (strstr(cmd, "F_GETFD"))
		ocmd = F_GETFD;
	else if (strstr(cmd, "F_SETFD"))
		ocmd = F_SETFD;
	else if (strstr(cmd, "F_GETFL"))
		ocmd = F_GETFL;
	else if (strstr(cmd, "F_SETFL"))
		ocmd = F_SETFL;
	else if (strstr(cmd, "F_GETLK"))
		ocmd = F_GETLK;
	else if (strstr(cmd, "F_SETLK"))
		ocmd = F_SETLK;
	else if (strstr(cmd, "F_SETLKW"))
		ocmd = F_SETLKW;
	else if (strstr(cmd, "F_SETOWN"))
		ocmd = F_SETOWN;
	else if (strstr(cmd, "F_GETOWN"))
		ocmd = F_GETOWN;
	else if (strstr(cmd, "F_SETSIG"))
		ocmd = F_SETSIG;
	else if (strstr(cmd, "F_GETSIG"))
		ocmd = F_GETSIG;
	else if (strstr(cmd, "F_SETOWN_EX"))
		ocmd = F_SETOWN_EX;
	else if (strstr(cmd, "F_GETOWN_EX"))
		ocmd = F_GETOWN_EX;
	else if (strstr(cmd, "F_GETOWNER_UIDS"))
		ocmd = F_GETOWNER_UIDS;

	/*
	 * sys_open() 
	 *
	 *    SYSCALL_DEFINE3(open, 
	 *                    const char __user *, filename, 
	 *                    int, flags,
	 *                    umode_t, mode)
	 */
	if (mode) {
		fd = syscall(__NR_open, path, oflags, omode);
	} else {
		fd = syscall(__NR_open, path, oflags);
	}
	if (fd < 0) {
		printf("Open: Can't open %s err %d\n", path, fd);
		return -1;
	}

	/*
	 * sys_fcntl
	 *
	 *    SYSCALL_DEFINE3(fcntl,
	 *                    unsigned int, fd,
	 *                    unsigned int, cmd,
	 *                    unsigned long, arg)
	 */
	syscall(__NR_fcntl, fd, ocmd, args);

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
