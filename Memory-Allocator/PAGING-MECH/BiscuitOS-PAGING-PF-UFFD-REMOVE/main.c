// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Userfaultfd on REMOVE
 *
 * (C) 2023.10.21 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
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

		/* Expect only EVENT REMOVE */
		if (msg.event != UFFD_EVENT_REMOVE)
			errExit("Unexpected event on userfaultfd");

		/* Display info about #PF event */
		printf("<UFFD> EVENT:   UFFD_EVENT_REMOVE\n");
		printf("<UFFD> FLAGS:   %#llx\n", msg.arg.pagefault.flags);
		printf("<UFDF> ADDRESS: %#llx\n", msg.arg.pagefault.address);
	}
}

int main()
{
	struct uffdio_register uffdio_register;
	struct uffdio_api uffdio_api;
	pthread_t thr;
	char *base;
	long uffd;

	/* Create and Enable userfaultfd object */
	uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
	if (uffd < 0)
		errExit("Userfaultfd create failed.");

	uffdio_api.api = UFFD_API;
	uffdio_api.features = UFFD_FEATURE_EVENT_REMOVE;
	if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1)
		errExit("IOCTL: UFFDIO_API");

	if ((uffdio_api.features & UFFD_FEATURE_EVENT_REMOVE) == 0)
		errExit("Don't Support UFFD_FEATURE_EVENT_REMOVE");

	/* Alloc Shared Memory */
	base = (char *)mmap((void *)MAP_ADDRESS, PAGE_SIZE,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED | MAP_ANONYMOUS,
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

	/* Write Ops, Trigger #PF */
	*base = 'D';
	printf("UFFD %#lx => %c\n", (unsigned long)base, *base);

	/* DROP PAGECACHE, and Trigger UFFD-REMOVE */
	madvise(base, PAGE_SIZE, MADV_REMOVE);

	munmap(base, PAGE_SIZE);
	close(uffd);

	return 0;
}
