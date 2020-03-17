/*
 * sys_pipe in C
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
/* __NR_pipe */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_pipe
#define __NR_pipe	42
#endif
#ifndef __NR_read
#define __NR_read	3
#endif
#ifndef __NR_write
#define __NR_write	4
#endif
#ifndef __NR_close
#define __NR_close	6
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_pipe helper\n");
	printf("Usage:\n");
	printf("      %s <-s send_string>\n", program_name);
	printf("\n");
	printf("\t-s\t--send\tSend string to another process.\n");
	printf("\ne.g:\n");
	printf("%s -s Hello_World\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *message = NULL;
	int c, hflags = 0;
	int fd[2], nbytes;
	pid_t pid;
	char buffer[128];
	opterr = 0;
	int ret;
	int *write_fd;
	int *read_fd;

	/* options */
	const char *short_opts = "hs:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "send_string", required_argument, NULL, 's'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 's': /* Send String */
			message = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !message) {
		usage(argv[0]);
		return 0;
	}

	write_fd = &fd[1];
	read_fd  = &fd[0];

	/*
	 * sys_pipe
	 *
	 *    SYSCALL_DEFINE1(pipe,
	 *                    int __user *, fildes)
	 */
	ret = syscall(__NR_pipe, fd);
	if (ret < 0) {
		printf("fail to create pipe\n");
		return -1;
	}

	/*
	 * sys_fork
	 *
	 *    SYSCALL_DEFINE0(fork)
	 */
	pid = syscall(__NR_fork);
	if (pid < 0) {
		printf("Fail to fork\n");
		return -1;
	}

	/* Send message */
	if (pid == 0) {
		/*
		 * sys_close
		 *
		 *    SYSCALL_DEFINE1(close,
		 *                    unsigned int, fd)
		 */
		syscall(__NR_close, *read_fd);
		/*
		 * sys_write
		 *
		 *    SYSCALL_DEFINE3(write,
		 *                    unsigned int, fd,
		 *                    const char __user *, buf,
		 *                    size_t, count)
		 */
		ret = syscall(__NR_write, *write_fd, message, strlen(message));
		if (ret == strlen(message))
			printf("Send %d bytes OK: %s\n", ret, message);
		else
			printf("Send fail %d: %s\n", ret, message);
		syscall(__NR_close, *write_fd);
		return 0;
	} else {
		/*
		 * sys_close
		 *
		 *    SYSCALL_DEFINE1(close,
		 *                    unsigned int, fd)
		 */
		syscall(__NR_close, *write_fd);
		/*
		 * sys_read
		 *
		 *    SYSCALL_DEFINE3(read,
		 *                    unsigned int, fd,
		 *                    char __user *, buf,
		 *                    size_t, count)
		 */
		nbytes = syscall(__NR_read, *read_fd, buffer, sizeof(buffer));
		if (nbytes < 0)
			printf("Recive fail %d\n", nbytes);
		else
			printf("Recvice %d bytes OK: %s\n", nbytes, buffer);
		syscall(__NR_close, *read_fd);
	}

	return 0;
}
