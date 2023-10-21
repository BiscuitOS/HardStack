// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault - Hugetlb Memory on MEMFD
 *
 * (C) 2023.10.19 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <fcntl.h>
#include <linux/memfd.h>

#define UNIX_FILE	"/tmp/BiscuitOS-UNIX"
#define MAP_SIZE	(2 * 1024 * 1024)
#define MAP_VADDR	(0x6000000000)

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
	int sockfd, memfd, afd;
	char *base;

	/* Setup UNIX-SOCKET */
	sockfd = socket_setup();

	/* CREATE Shared-File */
	memfd = memfd_create("BiscuitOS-HUGETLB", MFD_HUGETLB);
	if (memfd < 0) {
		perror(" |-> Client memfd error");
		goto err;
	}
	ftruncate(memfd, MAP_SIZE);

	/* Alloc Shared Memory with Shared-File */
	base = (char *)mmap((void *)MAP_VADDR, MAP_SIZE,
			   PROT_READ | PROT_WRITE,
			   MAP_SHARED | MAP_HUGETLB,
			   memfd,
			   0);
	if (base == MAP_FAILED) {
		perror(" |-> Client mmap failed");
		goto err_file;
	}
	sprintf(base, "Bello BiscuitOS");

	while (1) {
		/* accept */
		afd = accept(sockfd, NULL, NULL);
		if (afd < 0) {
			perror(" |-> Server Accept ERROR");
			goto err_unmap;
		}

		/* Send Shared file fd */
		send_fd(afd, memfd);
		close(afd);
	}

	munmap(base, MAP_SIZE);
	return 0;		

err_unmap:
	munmap(base, MAP_SIZE);
err_file:
	close(memfd);
err:
	close(sockfd);
	return -1;
}
