// SPDX-License-Identifier: GPL-2.0
/*
 * Copy-Page-Range: Userfaultfd WP
 *
 * (C) 2023.11.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <linux/userfaultfd.h>

#define PAGE_SIZE	(4 * 1024)
#define MAP_ADDRESS	(0x6000000000)
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

#ifndef UFFDIO_WRITEPROTECT_MODE_WP
#define UFFDIO_WRITEPROTECT_MODE_WP	(1<<0)

struct uffdio_writeprotect {
	struct uffdio_range range;
	unsigned long long mode;
};

#define _UFFDIO_WRITEPROTECT	(0x06)
/* userfaultfd ioctl ids */
#define UFFDIO			0xAA
#define UFFDIO_WRITEPROTECT	_IOWR(UFFDIO, _UFFDIO_WRITEPROTECT, \
					struct uffdio_writeprotect)
#endif

static void *fault_handler_thread(void *arg)
{
	struct uffd_msg msg;
	long uffd = (long)arg;

	/* LOOP, POOL */
	for (;;) {
		struct pollfd pollfd;
		int n;

		pollfd.fd = uffd;
		pollfd.events = POLLIN;
		n = poll(&pollfd, 1, -1);
		if (n < 0)
			errExit("Pool");

		/* Read an Event from the userfaultfd */
		n = read(uffd, &msg, sizeof(msg));
		if (n <= 0)
			errExit("EOF on userfaultfd!");

		/* Display info about #PF event */
		printf("<UFFD> EVENT:   %#x\n", msg.event);
		printf("<UFFD> FLAGS:   %#llx\n", msg.arg.pagefault.flags);
		printf("<UFFD> ADDRESS: %#llx\n", msg.arg.pagefault.address);

		if (msg.arg.pagefault.flags & UFFD_PAGEFAULT_FLAG_WP)
			printf("<UFFD> Dectect Write-Protection.\n");
	}
}

int main()
{
	struct uffdio_register uffdio_register;
	struct uffdio_writeprotect uffdio_wp;
	struct uffdio_api uffdio_api;
	char *base, ch;
	pthread_t thr;
	long uffd;

	/* Create and Enable userfaultfd object */
	uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
	if (uffd < 0)
		errExit("Userfaultfd create failed.");

	uffdio_api.api = UFFD_API;
	uffdio_api.features = UFFD_FEATURE_EVENT_FORK;
	if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1)
		errExit("IOCTL: UFFDIO_API");

	/* Alloc Anonymous Memory */
	base = (char *)mmap((void *)MAP_ADDRESS, PAGE_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS,
			    -1,
			    0);
	if (base == MAP_FAILED)
		errExit("MMAP Anonymous memory.");

	/* Register the memory range for uffd */
	uffdio_register.range.start = (unsigned long)base;
	uffdio_register.range.len = PAGE_SIZE;
	uffdio_register.mode = UFFDIO_REGISTER_MODE_WP;
	if (ioctl(uffd, UFFDIO_REGISTER, &uffdio_register) == -1)
		errExit("IOCTL: UFFDIO_REGISTER");

	/* Create a thread that will process the userfaultfd events */
	if (pthread_create(&thr, NULL, fault_handler_thread, (void *)uffd) != 0)
		errExit("Pthread Create.\n");

	/* Read Ops, Trigger #PF, don't trigger UFFD  */
	ch = *base;
	printf("UFFD %#lx => %c\n", (unsigned long)base, ch);

	/* WRITE-PROTECT */
	uffdio_wp.range.start = (unsigned long)base;
	uffdio_wp.range.len = PAGE_SIZE;
	uffdio_wp.mode = UFFDIO_WRITEPROTECT_MODE_WP;
	if (ioctl(uffd, UFFDIO_WRITEPROTECT, &uffdio_wp) == -1)
		errExit("IOCTL: UFFDIO_WRITEPROTECT");

	/* Copy-Page-Range */
	if (fork() == 0) {
		/* Son Write Ops, Trigger UFFD-WP */
		*base = 'B';
	} else {
		/* Father Write Ops, Trigger UFFD-WP */
		sleep(0.5);
		*base = 'D';
	}

	sleep(-1); /* Just For debug */
	munmap(base, PAGE_SIZE);
	close(uffd);

	return 0;
}
