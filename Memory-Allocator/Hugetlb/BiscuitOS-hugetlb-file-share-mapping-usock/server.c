/*
 * Hugetlb: Unix-Socket Share Memory on Server
 *
 * (C) 2022.01.12 <buddy.zhang@aliyun.com>
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

#define UNIX_FILE	"/mnt/BiscuitOS-hugetlbfs/BiscuitOS-UNIX"
/* Hugetlb */
#define HPAGE_SIZE	(2UL * 1024 * 1024)
#define BISCUITOS_SIZE	(2 * HPAGE_SIZE)

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
		char *base;

		/* accept */
		afd = accept(sockfd, NULL, NULL);
		if (afd < 0) {
			perror(" |-> Server Accept ERROR");
			goto err;
		}

		/* Recv fd */
		memfd = recv_fd(afd);
		base = (char *)mmap(NULL, BISCUITOS_SIZE,
					PROT_READ | PROT_WRITE,
					MAP_SHARED,
					memfd, 0);
		if (base == MAP_FAILED)
			perror(" |-> Server MMAP failed");
		printf(" |-> Serv Recv: %s\n", base);

		munmap(base, BISCUITOS_SIZE);		
		close(afd);
	}

err:
	close(sockfd);
	return 0;
}
