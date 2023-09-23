// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Shmem Memory on MEMFD
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
#include <linux/memfd.h>

#define UNIX_FILE	"BiscuitOS-UNIX"
#define MAP_SIZE	(4096)
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

	/* Create Shared Memory Area */
	memfd = memfd_create("BiscuitOS-SHMEM", 0);
	if (memfd < 0) {
		perror(" |-> Client memfd error");
		goto err;
	}
	ftruncate(memfd, MAP_SIZE);

	/* Alloc Shared Memory with MEMFD */
	base = (char *)mmap((void *)MAP_VADDR, MAP_SIZE,
			   PROT_READ | PROT_WRITE,
			   MAP_SHARED,
			   memfd, 0);
	if (base == MAP_FAILED) {
		perror(" |-> Client mmap failed");
		goto err;
	}

	/* Write Ops, Trigger #PF */
	*base = 'B';

	/* Send Buffer */
	sprintf(base, "Hello %s", "BiscuitOS");
	send_fd(sockfd, memfd);

	munmap(base, MAP_SIZE);
	close(memfd);
err:
	close(sockfd);
	return 0;
}
