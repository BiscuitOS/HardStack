/*
 * Hugetlb: UNIX-SOCK Shared Memory on Client
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
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define UNIX_FILE		"/mnt/BiscuitOS-hugetlbfs/BiscuitOS-UNIX"
/* Hugetlb */
#define HPAGE_SIZE		(2UL * 1024 * 1024)
#define BISCUITOS_SIZE		(2 * HPAGE_SIZE)
#define BISCUITOS_HUGEPAGE_PATH	"/mnt/BiscuitOS-hugetlbfs/hugepage"

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

int main(void)
{
	struct sockaddr_un un;
	int memfd, sockfd;
	char *base;

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

	/* Open hugepage file */
	memfd = open(BISCUITOS_HUGEPAGE_PATH, O_RDWR | O_CREAT);
	if (memfd < 0) {
		printf("ERROR: failed open %s\n", BISCUITOS_HUGEPAGE_PATH);
		return -EINVAL;
	}
	
	base = (char *)mmap(NULL, BISCUITOS_SIZE,
				PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_HUGETLB,
				memfd, 0);
	if (base == MAP_FAILED) {
		perror(" |-> Client mmap failed");
		goto err;
	}

	/* Send Buffer */
	sprintf(base, "Hello %s", "BiscuitOS");
	send_fd(sockfd, memfd);

	munmap(base, BISCUITOS_SIZE);
	close(memfd);
err:
	close(sockfd);
	return 0;
}
