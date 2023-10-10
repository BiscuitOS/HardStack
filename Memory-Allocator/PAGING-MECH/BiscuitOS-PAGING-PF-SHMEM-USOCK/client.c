// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Shmem Memory on USOCK
 *
 * (C) 2023.09.22 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)
#define UNIX_FILE	"/tmp/BiscuitOS-UNIX"

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

static int sock_setup(void)
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

	memfd = recv_fd(sockfd);
	if (memfd < 0) {
		perror("Recv memfd error");
		goto err;
	}

	return memfd;
err:
	close(sockfd);
	exit(-1);
}

int main(void)
{
	int memfd;
	char *base;

	/* Setup Unix-Socket and Receive Shared-File */
	memfd = sock_setup();

	/* Mmap Shared Memory into Address-Space */
	base = (char *)mmap((void *)MAP_VADDR, MAP_SIZE,
			   PROT_READ | PROT_WRITE,
			   MAP_SHARED,
			   memfd,
			   0);
	if (base == MAP_FAILED) {
		perror("Mapping Failed on Client.\n");
		goto err;
	}

	/* Write Ops, Trigger #PF */
	*base = 'H';
	/* Read Ops, Don't Trigger #PF */
	printf("UNIX-SOCKET Client %d: %#lx => %s\n", 
				getpid(), (unsigned long)base, base);

	munmap(base, MAP_SIZE);
	close(memfd);
	return 0;
	
err:
	close(memfd);
	return -1;
}
