/*
 * sys_munmap in C
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
/* __NR_open/__NR_mmap2 */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
/* mmap flags */
#include <sys/mman.h>

/* Architecture defined */
#ifndef __NR_open
#define __NR_open	5
#endif
#ifndef __NR_close
#define __NR_close	6
#endif
#ifndef __NR_mmap2
#define __NR_mmap2	192
#endif
#ifndef __NR_munmap
#define __NR_munmap	91
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
#ifndef PROT_SEM
#define PROT_SEM		0x8	/* page may be used for atomic ops */
#endif
#ifndef MAP_SHARED_VALIDATE
#define MAP_SHARED_VALIDATE	0x03	/* share + validate extension flags */
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED	0x4000000	/* For anonymous mmap, memory
						 * could be uninitialized */
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_munmap helper\n");
	printf("Usage:\n");
	printf("      %s <-p pathname> <-f flags> <-m mode> "
			"<-P prot_flags> <-F map_flags> "
			"<-o offset> <-a address> <-s size>\n", program_name);
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
	printf("\t-P\t--Prot_flags\tThe flags for PROT.\n");
	printf("\t\t\tPROT_READ        page can be read\n");
	printf("\t\t\tPROT_WRITE       page can be written\n");
	printf("\t\t\tPROT_EXEC        page can be executed\n");
	printf("\t\t\tPROT_SEM         page may be used for atomic ops\n");
	printf("\t\t\tPROT_NONE        page can not be accessed\n");
	printf("\t\t\tPROT_GROWSDOWN   mprotect flag: extend change\n");
	printf("\t\t\t                     to start of growsdown vma\n");
	printf("\t\t\tPROT_GROWSUP     mprotect flag: extend change\n");
	printf("\t\t\t                 to end of growsup vma\n");
	printf("\t-F\t--mmap_flags\tThe flags for mapping.\n");
	printf("\t\t\tMAP_SHARED       Share changes\n");
	printf("\t\t\tMAP_PRIVATE      Changes are private\n");
	printf("\t\t\tMAP_SHARED_VALIDATE\n");
	printf("\t\t\t                 share + validate extension flags\n");
	printf("\t\t\tMAP_TYPE         Mask for type of mapping\n");
	printf("\t\t\tMAP_FIXED        Interpret addr exactly\n");
	printf("\t\t\tMAP_ANONYMOUS    don't use a file\n");
	printf("\t\t\tMAP_UNINITIALIZED\n");
	printf("\t\t\t                 For anonymous mmap, memory could\n");
	printf("\t\t\t                    be uninitialized\n");
	printf("\t-o\t--offset\tThe offset for mapping.\n");
	printf("\t-a\t--address\tThe address for mapping.\n");
	printf("\t-s\t--size\tThe size for mapping.\n");
	printf("\ne.g:\n");
	printf("%s -p BiscuitOS_file -f O_RDWR,O_CREAT -m S_IRUSR,S_IWUSR "
			"-s 100 -o 0 -a 0 -P PROT_READ "
			"-F MAP_PRIVATE\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *path = NULL;
	char *mode = NULL;
	char *flags = NULL;
	char *prot = NULL;
	char *mmapf = NULL;
	unsigned long addr = 0;
	unsigned long offset = 0;
	unsigned long size = 0;
	unsigned long prot_flags = 0;
	unsigned long mmap_flags = 0;
	mode_t omode = 0;
	int mode_value;
	int c, hflags = 0;
	int oflags = 0;
	int fd;
	opterr = 0;
	void *base;

	/* options */
	const char *short_opts = "hp:f:m:a:s:o:P:F:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "path", required_argument, NULL, 'p'},
		{ "flags", required_argument, NULL, 'f'},
		{ "mode", required_argument, NULL, 'm'},
		{ "address", required_argument, NULL, 'a'},
		{ "size", required_argument, NULL, 's'},
		{ "offset", required_argument, NULL, 'o'},
		{ "PROT", required_argument, NULL, 'P'},
		{ "mmap_flags", required_argument, NULL, 'F'},
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
		case 'a': /* address */
			sscanf(optarg, "%ld", &addr);
			break;
		case 's': /* size */
			sscanf(optarg, "%ld", &size);
			break;
		case 'o': /* offset */
			sscanf(optarg, "%ld", &offset);
			break;
		case 'P': /* PROT */
			prot = optarg;
			break;
		case 'F': /* MMAP_flags */
			mmapf = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !path || !flags || !mode || !size || !prot || !mmapf) {
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

	/* parse PROT flags */
	if (strstr(prot, "PROT_READ"))
		prot_flags |= PROT_READ;
	if (strstr(prot, "PROT_WRITE"))
		prot_flags |= PROT_WRITE;
	if (strstr(prot, "PROT_EXEC"))
		prot_flags |= PROT_EXEC;
	if (strstr(prot, "PROT_SEM"))
		prot_flags |= PROT_SEM;
	if (strstr(prot, "PROT_NONE"))
		prot_flags |= PROT_NONE;
	if (strstr(prot, "PROT_GROWSDOWN"))
		prot_flags |= PROT_GROWSDOWN;
	if (strstr(prot, "PROT_GROWSUP"))
		prot_flags |= PROT_GROWSUP;

	/* parse mmap flags */
	if (strstr(mmapf, "MAP_SHARED"))
		mmap_flags |= MAP_SHARED;
	if (strstr(mmapf, "MAP_PRIVATE"))
		mmap_flags |= MAP_PRIVATE;
	if (strstr(mmapf, "MAP_SHARED_VALIDATE"))
		mmap_flags |= MAP_SHARED_VALIDATE;
	if (strstr(mmapf, "MAP_TYPE"))
		mmap_flags |= MAP_TYPE;
	if (strstr(mmapf, "MAP_FIXED"))
		mmap_flags |= MAP_FIXED;
	if (strstr(mmapf, "MAP_ANONYMOUS"))
		mmap_flags |= MAP_ANONYMOUS;
	if (strstr(mmapf, "MAP_UNINITIALIZED"))
		mmap_flags |= MAP_UNINITIALIZED;

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
	 * sys_mmap2
	 *
	 *    asmlinkage long sys_mmap2(
	 *              unsigned long addr, unsigned long len,
	 *              unsigned long prot, unsigned long flags,
	 *              unsigned long fd, unsigned long pgoff)
	 */
	base = (void *)(unsigned long)syscall(__NR_mmap2, size, 
					prot_flags, mmap_flags, fd, offset);
	if (base == MAP_FAILED) {
		printf("__NR_mmap2 failed\n");

		/*
		 * sys_close
		 *
	 	 *    SYSCALL_DEFINE1(close,
		 *                    unsigned int, fd)
		 *
		 */
		syscall(__NR_close, (unsigned int)fd);
		return -1;
	}

	printf("Mapping base address: %#lx\n", (unsigned long)base);

	/*
	 * sys_munmap
	 *
	 *    SYSCALL_DEFINE2(munmap,
	 *                    unsigned long, addr,
	 *                    size_t, len)
	 */
	syscall(__NR_munmap, base, size);

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
