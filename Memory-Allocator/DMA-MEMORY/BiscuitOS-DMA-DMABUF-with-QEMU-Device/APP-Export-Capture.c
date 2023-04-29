/*
 * DMABUF Export on Userspace
 *
 * (C) 2023.02.25 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define UNIX_FILE	"/tmp/BiscuitOS-UNIX"
#define DMABUF		"/dev/BiscuitOS-DMABUF-Export"
/* IOCTL */
#define BISCUITOS_IO		0xBD
#define BISCUITOS_PCI_TO_MEM	_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_MEM_TO_PCI	_IO(BISCUITOS_IO, 0x01)
#define BISCUITOS_DMABUF_FD	_IO(BISCUITOS_IO, 0x02)

static void send_fd(int sockfd, int fd)
{
	struct msghdr msg = {0};
	struct cmsghdr *cmsg;
	char buf[CMSG_SPACE(sizeof(int))], data;
	struct iovec io = { 
		.iov_base = &data,
		.iov_len = 1,
	};

	msg.msg_iov = &io;
	msg.msg_iovlen = 1;
	msg.msg_control = buf;
	msg.msg_controllen = sizeof(buf);

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type  = SCM_RIGHTS;
	cmsg->cmsg_len   = CMSG_LEN(sizeof(int));

	memcpy((int *)CMSG_DATA(cmsg), &fd, sizeof(int));

	if (sendmsg(sockfd, &msg, 0) < 0)
		perror(" |-> Client sendmsg ERROR");
}

static int socket_setup(void)
{
	struct sockaddr_un un;
	int sockfd;

	/* UNIX socket */
	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror(" |-> Server SOCKET ERROR.\n");
		goto err;
	}

	un.sun_family = AF_UNIX;
	unlink(UNIX_FILE);
	strcpy(un.sun_path, UNIX_FILE);

	/* Bind */
	if (bind(sockfd, (struct sockaddr *)&un, sizeof(un)) < 0) {
		perror(" |-> Server Bind ERROR.\n");
		goto err;
	}

	/* listen */
	if (listen(sockfd, 2) < 0) {
		perror(" |-> Server Listen ERROR.");
		goto err;
	}

	return sockfd;
err:
	exit(-1);
}

int main(void)
{
	int fd, dmabuf_fd;
	static int execute_once = 0;
	int sockfd = socket_setup();

	/* OPEN */
	fd = open(DMABUF, O_RDONLY);
	if (fd < 0) {
		printf("ERROR: Open %s Failed\n", DMABUF);
		goto err;
	}
	ioctl(fd, BISCUITOS_DMABUF_FD, &dmabuf_fd);

	while (1) {
		int afd, memfd;

		/* accept */
		afd = accept(sockfd, NULL, NULL);
		if (afd < 0) {
			perror(" |-> Server Accept ERROR");
			goto err;
		}

		send_fd(afd, dmabuf_fd);
		/* DMA: Video Capture to DDR */
		ioctl(fd, BISCUITOS_PCI_TO_MEM, 0);

		close(afd);
	}
	close(fd);
err:
	close(sockfd);
	return 0;
}
