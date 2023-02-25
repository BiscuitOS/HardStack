/*
 * DMABUF Import on Userspace
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
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>

#define UNIX_FILE		"/tmp/BiscuitOS-UNIX"
#define DMABUF			"/dev/BiscuitOS-DMABUF-import"
/* IOCTL */
#define BISCUITOS_IO		0xBD
#define BISCUITOS_DMABUF_FD	_IO(BISCUITOS_IO, 0x00)

static int recv_fd(int socket)
{
	struct msghdr msg = {0};
	struct cmsghdr *cmsg;
	char buf[CMSG_SPACE(sizeof(int))], data;
	int fd;
	struct iovec io = {
		.iov_base = &data,
		.iov_len = 1,
	};

	msg.msg_iov = &io;
	msg.msg_iovlen = 1;
	msg.msg_control = buf;
	msg.msg_controllen = sizeof(buf);

	if (recvmsg(socket, &msg, 0) < 0)
		perror(" |-> Server Recv mesg error");

	cmsg = CMSG_FIRSTHDR(&msg);
	memcpy(&fd, (int *)CMSG_DATA(cmsg), sizeof(int));

	return fd;
}

int main(void)
{
	struct sockaddr_un un;
	int memfd, sockfd, fd;
	void *base;

	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, UNIX_FILE);

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror(" |-> Client Sock ERROR");
		exit(-1);
	}

	if (connect(sockfd, (struct sockaddr *)&un, sizeof(un)) < 0) {
		perror(" |-> Client connect ERROR");
		goto err;
	}

	/* Obtain dmabuf */
	memfd = recv_fd(sockfd);
	if (memfd < 0) {
		perror("Recv memfd error");
		goto err;
	}
	/* Translate dmabuf to driver */
	fd = open(DMABUF, O_RDWR);
	if (fd < 0) {
		printf("Open %s failed.\n", DMABUF);
		goto err;
	}
	ioctl(fd, BISCUITOS_DMABUF_FD, &memfd);

	/* MMAP DMA Memory */
	base = mmap(NULL, 4096,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			0);
	if (base == MAP_FAILED) {
		perror("Mapping failed.\n");
		goto err_mmap;
	}
	sleep(4);
	printf("IMPORT0: %s\n", (char *)base);
	sprintf((char *)base, "Hello BiscuitOS DMABUF!");

err_mmap:
	close(fd);
err:
	close(sockfd);
	return 0;
}
