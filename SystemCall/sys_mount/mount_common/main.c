/*
 * sys_mount in C
 *
 * (C) 2020.03.13 BuddyZhang1 <buddy.zhang@aliyun.com>
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
/* mount flags */
#include <sys/mount.h>
/* __NR_mount */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_mount helper\n");
	printf("Usage:\n");
	printf("      %s <-n dev_name> <-d dir_name> <-t filesystem_type> "
	                 "<-f mount_flags> [-o options]\n", program_name);
	printf("\n");
	printf("\t-n\t--dev_name   The mount device name.\n");
	printf("\t-d\t--dir_name   The mount dirent/pointer.\n");
	printf("\t-t\t--fs_type    The filesystem type.\n");
	printf("\t-f\t--flags      The mount flags.\n");
	printf("\t\t\tMS_RDONLY       Mount read-only\n");
	printf("\t\t\tMS_NOSUID       Ignore suid and sgid bits\n");
	printf("\t\t\tMS_NODEV        Disallow access to device special files\n");
	printf("\t\t\tMS_NOEXEC       Disallow program execution\n");
	printf("\t\t\tMS_SYNCHRONOUS  Writes are synced at once\n");
	printf("\t\t\tMS_REMOUNT      Alter flags for a mounted FS.\n");
	printf("\t\t\tMS_MANDLOCK     Allow mandatory locks on an FS\n");
	printf("\t\t\tMS_DIRSYNC      Directory modifications are synchronous\n");
	printf("\t\t\tMS_NOATIME      Do not update access times\n");
	printf("\t\t\tMS_NODIRATIME   Do not update directory access times\n");
	printf("\t\t\tMS_BIND         Bind directory at different place\n");
	printf("\t\t\tMS_MOVE\n");
	printf("\t\t\tMS_REC\n");
	printf("\t\t\tMS_SILENT\n");
	printf("\t\t\tMS_POSIXACL     VFS does not apply the umask\n");
	printf("\t\t\tMS_UNBINDABLE   Change to private\n");
	printf("\t\t\tMS_PRIVATE      Change to private\n");
	printf("\t\t\tMS_SLAVE        Change to slave\n");
	printf("\t\t\tMS_SHARED       Change to shared\n");
	printf("\t\t\tMS_RELATIME     Update atime relative to mtime/ctime\n");
	printf("\t\t\tMS_KERNMOUNT    This is a kern_mount call\n");
	printf("\t\t\tMS_I_VERSION    Update inode I_version field.\n");
	printf("\t\t\tMS_STRICTATIME  Always perform atime updates.\n");
	printf("\t\t\tMS_LAZYTIME     Update the on-disk [acm]times lazily\n");
	printf("\t\t\tMS_ACTIVE\n");
	printf("\t\t\tMS_NOUSER\n");
	printf("\t-o\t--options    The mount options.\n");
	printf("\t\t\t\n");
	printf("\ne.g:\n");
	printf("%s -n BiscuitOS_tmpfs -d /tmpfs_bs -t tmpfs_bs "
			"-f MS_SILENT,MS_NOATIME -o size=8M\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *dev_name = NULL;
	char *dir_name = NULL;
	char *fs_type = NULL;
	char *flags = NULL;
	char *options = NULL;
	unsigned long omode = 0;
	int c, hflags = 0;
	int oflags = 0;
	int fd;
	opterr = 0;

	/* options */
	const char *short_opts = "hn:d:t:f:o:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "dev_name", required_argument, NULL, 'n'},
		{ "dir_name", required_argument, NULL, 'd'},
		{ "fs_type", required_argument, NULL, 't'},
		{ "flags", required_argument, NULL, 'f'},
		{ "options", required_argument, NULL, 'o'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'n': /* mount device */
			dev_name = optarg;
			break;
		case 'd': /* mount direct/pointer */
			dir_name = optarg;
			break;
		case 't': /* filesystem type */
			fs_type = optarg;
			break;
		case 'f': /* mount flags */
			flags = optarg;
			break;
		case 'o': /* options */
			options = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !dev_name || !dir_name || !fs_type || !flags) {
		usage(argv[0]);
		return 0;
	}

	/* parse mount flags argument */
	if (strstr(flags, "MS_RDONLY"))
		oflags |= MS_RDONLY;
	if (strstr(flags, "MS_NOSUID"))
		oflags |= MS_NOSUID;
	if (strstr(flags, "MS_NODEV"))
		oflags |= MS_NODEV;
	if (strstr(flags, "MS_NOEXEC"))
		oflags |= MS_NOEXEC;
	if (strstr(flags, "MS_SYNCHRONOUS"))
		oflags |= MS_SYNCHRONOUS;
	if (strstr(flags, "MS_REMOUNT"))
		oflags |= MS_REMOUNT;
	if (strstr(flags, "MS_MANDLOCK"))
		oflags |= MS_MANDLOCK;
	if (strstr(flags, "MS_DIRSYNC"))
		oflags |= MS_DIRSYNC;
	if (strstr(flags, "MS_NOATIME"))
		oflags |= MS_NOATIME;
	if (strstr(flags, "MS_NODIRATIME"))
		oflags |= MS_NODIRATIME;
	if (strstr(flags, "MS_BIND"))
		oflags |= MS_BIND;
	if (strstr(flags, "MS_MOVE"))
		oflags |= MS_MOVE;
	if (strstr(flags, "MS_REC"))
		oflags |= MS_REC;
	if (strstr(flags, "MS_SILENT"))
		oflags |= MS_SILENT;
	if (strstr(flags, "MS_POSIXACL"))
		oflags |= MS_POSIXACL;
	if (strstr(flags, "MS_UNBINDABLE"))
		oflags |= MS_UNBINDABLE;
	if (strstr(flags, "MS_PRIVATE"))
		oflags |= MS_PRIVATE;
	if (strstr(flags, "MS_SLAVE"))
		oflags |= MS_SLAVE;
	if (strstr(flags, "MS_SHARED"))
		oflags |= MS_SHARED;
	if (strstr(flags, "MS_RELATIME"))
		oflags |= MS_RELATIME;
	if (strstr(flags, "MS_KERNMOUNT"))
		oflags |= MS_KERNMOUNT;
	if (strstr(flags, "MS_I_VERSION"))
		oflags |= MS_I_VERSION;
	if (strstr(flags, "MS_STRICTATIME"))
		oflags |= MS_STRICTATIME;
	if (strstr(flags, "MS_LAZYTIME"))
		oflags |= MS_LAZYTIME;
	if (strstr(flags, "MS_ACTIVE"))
		oflags |= MS_ACTIVE;
	if (strstr(flags, "MS_NOUSER"))
		oflags |= MS_NOUSER;

	/*
	 * sys_mount
	 *
	 *    SYSCALL_DEFINE5(mount,
	 *                    char __user *, dev_name,
	 *                    char __user *, dir_name,
	 *                    char __user *, type,
	 *                    unsigned long, flags,
	 *                    void __user *, data)
	 */
	syscall(__NR_mount, dev_name, dir_name, fs_type, oflags, options);
	return 0;
}
