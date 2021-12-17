/*
 * Hugetlb: memfd_create Shared Memory on Client
 *
 * (C) 2021.12.03 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifdef __i386__
#error "Process doesn't support I386 Architecture"
#endif
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

#define UNIX_FILE		"BiscuitOS-UNIX"
/* Hugetlb */
#define HPAGE_SIZE		(2 * 1024 * 1024)
#define BISCUITOS_SIZE		(2 * HPAGE_SIZE)

#ifndef MFD_HUGETLB
#define MFD_HUGETLB		0x0004U
#endif

#ifndef __NR_memfd_create
#ifdef __aarch64__
#define __NR_memfd_create	279
#elif __x86_64__
#define __NR_memfd_create	319
#endif
#endif

#ifndef MFD_HUGE_2MB
#define HUGETLB_FLAG_ENCODE_SHIFT	26
#define MFD_HUGE_2MB			(21 << HUGETLB_FLAG_ENCODE_SHIFT)
#endif

#ifndef MAP_HUGE_2MB
#define HUGETLB_FLAG_ENCODE_SHIFT	26
#define MAP_HUGE_2MB			(21 << HUGETLB_FLAG_ENCODE_SHIFT)
#endif


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

	/* Create Shared Memory Area: invoke memfd_create  */
	memfd = syscall(__NR_memfd_create, "BiscuitOS-mem", 
				MFD_HUGETLB | MFD_HUGE_2MB);
	if (memfd < 0) {
		perror(" |-> Client memfd error");
		goto err;
	}
	ftruncate(memfd, BISCUITOS_SIZE);
	
	base = (char *)mmap(NULL, BISCUITOS_SIZE,
				PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_HUGETLB |
				MAP_HUGE_2MB,
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
