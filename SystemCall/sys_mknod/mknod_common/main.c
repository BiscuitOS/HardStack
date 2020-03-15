/*
 * sys_mknod in C
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
/* __NR_mknod */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
/* makedev */
#include <sys/sysmacros.h>

/* Architecture defined */
#ifndef __NR_mknod
#define __NR_mknod	14
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_mknod helper\n");
	printf("Usage:\n");
	printf("      %s <-d devname> <-m mode> <-t node_type> "
				"<-M major> <-N minor>\n", program_name);
	printf("\n");
	printf("\t-d\t--devname     The device node name.\n");
	printf("\t-m\t--mode        The mode for opening.\n");
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
	printf("\t-t\t--node_type   The node type.\n");
	printf("\t\t\tS_IFMT\n");
	printf("\t\t\tS_IFREG    The regular file\n");
	printf("\t\t\tS_IFBLK    The block device\n");
	printf("\t\t\tS_IFDIR    The dentroy\n");
	printf("\t\t\tS_IFCHR    The character device\n");
	printf("\t\t\tS_IFIFO    The FIFO\n");
	printf("\t\t\tS_IFLNK    The symbolic link\n");
	printf("\t-M\t--Major       The Major number.\n");
	printf("\t-N\t--Minor       The Minor number.\n");
	printf("\ne.g:\n");
	printf("%s -d /dev/BiscuitOS_node -m S_IRUSR,S_IWUSR "
			"-t S_IFCHR -M 128 -N 210\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *devname = NULL;
	char *type = NULL;
	char *mode = NULL;
	mode_t omode = 0;
	int major, minor;
	int c, hflags = 0;
	int node_type = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hd:m:t:M:N:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "devname", required_argument, NULL, 'd'},
		{ "node_type", required_argument, NULL, 't'},
		{ "major", required_argument, NULL, 'M'},
		{ "minor", required_argument, NULL, 'N'},
		{ "mode", required_argument, NULL, 'm'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'd': /* node_name */
			devname = optarg;
			break;
		case 't': /* node_type */
			type = optarg;
			break;
		case 'm': /* mode */
			mode = optarg;
			break;
		case 'M': /* Major */
			sscanf(optarg, "%d", &major);
			break;
		case 'N': /* Minor */
			sscanf(optarg, "%d", &minor);
			break;
		default:
			abort();
		}
	}

	if (hflags || !devname || !type || !mode || !major || !minor) {
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

	/* parse type argument */
	if (type) {
		if (strstr(type, "S_IFMT"))
			node_type |= S_IFMT;
		if (strstr(type, "S_IFREG"))
			node_type |= S_IFREG;
		if (strstr(type, "S_IFBLK"))
			node_type |= S_IFBLK;
		if (strstr(type, "S_IFDIR"))
			node_type |= S_IFDIR;
		if (strstr(type, "S_IFCHR"))
			node_type |= S_IFCHR;
		if (strstr(type, "S_IFIFO"))
			node_type |= S_IFIFO;
		if (strstr(type, "S_IFLNK"))
			node_type |= S_IFLNK;
	}

	/*
	 * sys_mknod
	 *
	 *    SYSCALL_DEFINE3(mknod,
	 *                    const char __user *, filename,
	 *                    umode_t, mode,
	 *                    unsigned, dev)
	 */
	syscall(__NR_mknod, devname, omode | node_type, makedev(major, minor));
	return 0;
}
