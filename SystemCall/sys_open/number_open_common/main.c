/*
 * sys_open: open numbers files
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
/* syscall() */
#include <unistd.h>
#include <errno.h>

/* Architecture syscall-no defined */
#ifndef __NR_open
#ifdef __x86_64__ /* X86_64 Special */
#define __NR_open	2
#else /* ARM32/ARM64/i386/RISCV32/RISCV64 */
#define __NR_open	5
#endif
#endif

#ifndef __NR_close
#ifdef __x86_64__ /* X86_64 Special */
#define __NR_close	3
#else /* ARM32/ARM64/i386/RISCV32/RISCV64 */
#define __NR_close	6
#endif
#endif

/* Architecture open-flags */
#ifndef O_ACCMODE
#define O_ACCMODE		00000003
#endif

#ifndef O_RDONLY
#define O_RDONLY		00000000
#endif

#ifndef O_WRONLY
#define O_WRONLY		00000001
#endif

#ifndef O_RDWR
#define O_RDWR			00000002
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC		02000000	/* set close_on_exec */
#endif

#ifndef O_DIRECTORY
#if defined __arm__ || __aarch64__ /* ARM32/ARM64 Special */
#define O_DIRECTORY		040000		/* must be a directory */
#else /* i386/x86_64/RISCV32/RISCV64 Special */
#define O_DIRECTORY		00200000	/* must be a directory */
#endif
#endif

#ifndef O_NOFOLLOW
#if defined __arm__ || __aarch64__ /* ARM32/ARM64 Special */
#define O_NOFOLLOW		0100000		/* don't follow links */
#else /* i386/x86_64/RISCV32/RISCV64 Special */
#define O_NOFOLLOW		00400000	/* don't follow links */
#endif
#endif

#ifndef O_CREAT
#define O_CREAT			00000100	/* not fcntl */
#endif

#ifndef O_EXCL
#define O_EXCL			00000200	/* not fcntl */
#endif

#ifndef O_NOCTTY
#define O_NOCTTY		00000400	/* not fcntl */
#endif

#ifndef O_TRUNC
#define O_TRUNC			00001000	/* not fcntl */
#endif

#ifndef O_APPEND
#define O_APPEND		00002000
#endif

/* ARM64/i386/x86_64/RISCV32/RISCV64 undefine */
#ifndef O_ASYNC
#define O_ASYNC			020000
#endif

#ifndef O_DSYNC
#define O_DSYNC			00010000	/* used to be O_SYNC, see below */
#endif

#ifndef __O_TMPFILE
#define __O_TMPFILE		020000000
#endif

#ifndef O_DIRECT
#if defined __arm__ || __aarch64__ /* ARM32/ARM64 Special */
#define O_DIRECT		0200000		/* direct disk access hint - currently ignored */
#else /* i386/x86_64/RISCV32/RISCV64 Special */
#define O_DIRECT		00040000	/* direct disk access hint - currently ignored */
#endif
#endif

#ifndef O_LARGEFILE
#if defined __arm__ || __aarch64__ /* ARM32/ARM64 Special */
#define O_LARGEFILE		0400000
#else /* i386/x86_64/RISCV32/RISCV64 Special */
#define O_LARGEFILE		00100000
#endif
#endif

/* ARM64/i386/X86_64/RISCV32/RISCV64 undefine */
#ifndef O_NATIME
#define O_NATIME		01000000
#endif

#ifndef O_NONBLOCK
#define O_NONBLOCK		00004000
#endif

#ifndef __O_SYNC
#define __O_SYNC		04000000
#endif

#ifndef O_PATH
#define O_PATH			010000000
#endif

/* Architecture Debug stub */
#ifndef __NR_debug_BiscuitOS
/* ARM32 */
#ifdef __arm__
#define __NR_debug_BiscuitOS    400
/* ARM64 */
#elif __aarch64__
#define __NR_debug_BiscuitOS    400
/* Intel i386 */
#elif __i386__
#define __NR_debug_BiscuitOS    387
/* Intel X64 */
#elif __x86_64__
#define __NR_debug_BiscuitOS    548
/* RISCV32 */
#elif __riscv_xlen == 32
#define __NR_debug_BiscuitOS    258
/* RISCV64 */
#elif __riscv_xlen == 64
#define __NR_debug_BiscuitOS    258
#endif
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: opne_number helper\n");
	printf("Usage:\n");
	printf("      %s <-n num> [-d debug] <-f flags> <-m mode>\n", program_name);
	printf("\n");
	printf("\t-n\t--num\tThe number for opening files.\n");
	printf("\t-d\t--debug\tDebug special number opening.\n");
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
	printf("\t\t\t__O_TMPFILE\n");
	printf("\t\t\tO_TRUNC\n");
	printf("\t\t\tO_APPEND\n");
	printf("\t\t\tO_ASYNC\n");
	printf("\t\t\tO_DIRECT\n");
	printf("\t\t\tO_DSYNC\n");
	printf("\t\t\tO_LARGEFILE\n");
	printf("\t\t\tO_NATIME\n");
	printf("\t\t\tO_NONBLOCK\n");
	printf("\t\t\t__O_SYNC\n");
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
	printf("%s -n 2 -d 0 -f O_RDWR,O_CREAT "
			"-m S_IRUSR,S_IRGRP\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *mode = NULL;
	char *flags = NULL;
	mode_t omode = 0;
	int mode_value;
	int c, hflags = 0;
	int oflags = 0;
	int number = -1;
	int *fd;
	int index;
	int debug_fd = 0;
	char path[64];
	opterr = 0;

	/* options */
	const char *short_opts = "hn:f:m:d:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "num", required_argument, NULL, 'n'},
		{ "debug", required_argument, NULL, 'd'},
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
		case 'n': /* Number */
			sscanf(optarg, "%d", &number);
			break;
		case 'd': /* Debug */
			sscanf(optarg, "%d", &debug_fd);
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

	if (hflags || number < 0 || !flags || !mode) {
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
	if (strstr(flags, "__O_TMPFILE"))
		oflags |= __O_TMPFILE;
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
	if (strstr(flags, "__O_SYNC"))
		oflags |= __O_SYNC;
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

	/* alloc fd-table */
	fd = malloc(sizeof(int) * number);

	for (index = 0; index < number; index++) {
		/* create filename */
		sprintf(path, "BiscuitOS-%d", index);

		/* Open and trace debug */
		if (debug_fd == index)
			syscall(__NR_debug_BiscuitOS, 1);
		/*
		 * sys_open() 
		 *
		 *    SYSCALL_DEFINE3(open, 
		 *                    const char __user *, filename, 
		 *                    int, flags,
		 *                    umode_t, mode)
		 */
		if (mode) {
			fd[index] = syscall(__NR_open, path, oflags, omode);
		} else {
			fd[index] = syscall(__NR_open, path, oflags);
		}
		/* Close debug */
		if (debug_fd == index)
			syscall(__NR_debug_BiscuitOS, 0);

		if (fd[index] < 0) {
			printf("Open[%d]: Can't open %s err %d\n", 
							index, path, errno);
			goto out;
		}
		printf("%s Open-fd: %d\n", path, fd[index]);
	}

out:
	while (index--) {
		/*
		 * sys_close()
		 *
		 *    SYSCALL_DEFINE1(close,
		 *                    unsigned int, fd)
		 *
		 */
		syscall(__NR_close, (unsigned int)fd[index]);	
	}

	free(fd);
	return 0;
}
