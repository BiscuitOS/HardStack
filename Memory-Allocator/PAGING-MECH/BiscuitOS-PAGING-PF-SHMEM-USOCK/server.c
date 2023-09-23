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
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define UNIX_FILE	"/tmp/BiscuitOS-UNIX"
#define MAP_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)

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
	int sockfd;

	printf("** Unix Socket **\n");
	/* UNIX socket */
	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror(" |-> Server SOCKET ERROR.\n");
		exit(-1);
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

	while (1) {
		struct sockaddr_un client_addr;
		int afd, memfd;
		char *base, ch;

		/* accept */
		afd = accept(sockfd, NULL, NULL);
		if (afd < 0) {
			perror(" |-> Server Accept ERROR");
			goto err;
		}

		/* Recv fd */
		memfd = recv_fd(afd);
		/* Alloc Share Memory */
		base = (char *)mmap((void *)MAP_VADDR, MAP_SIZE,
				    PROT_READ | PROT_WRITE,
				    MAP_SHARED,
				    memfd, 0);
		if (base == MAP_FAILED)
			perror(" |-> Server MMAP failed");

		/* Read ops, Trigger #PF */
		ch = *base;

		printf("SHMEM-USOCK: %#lx => %s(%c)\n", 
					(unsigned long)base, base, ch);

		munmap(base, MAP_SIZE);		
		close(afd);
	}

err:
	close(sockfd);
	return 0;
}
