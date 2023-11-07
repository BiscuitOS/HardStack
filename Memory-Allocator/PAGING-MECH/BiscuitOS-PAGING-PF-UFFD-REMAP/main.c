// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Userfaultfd on REMAP
 *
 * (C) 2023.10.22 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#define MAP_NEWADDR	(0x7000000000)
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

		/* Expect only EVENT REMAP */
		if (msg.event != UFFD_EVENT_REMAP)
			errExit("Unexpected event on userfaultfd");

		/* Display info about #PF event */
		printf("EVENT: UFFD_EVENT_REMAP\n");
		printf("FLAGS:   %#llx\n", msg.arg.pagefault.flags);
		printf("ADDRESS: %#llx\n", msg.arg.pagefault.address);
	}
}

int main()
{
	struct uffdio_register uffdio_register;
	struct uffdio_api uffdio_api;
	char *base, *rebase;
	pthread_t thr;
	long uffd;

	/* Create and Enable userfaultfd object */
	uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
	if (uffd < 0)
		errExit("Userfaultfd create failed.");

	uffdio_api.api = UFFD_API;
	uffdio_api.features = UFFD_FEATURE_EVENT_REMAP;
	if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1)
		errExit("IOCTL: UFFDIO_API");

	if ((uffdio_api.features & UFFD_FEATURE_EVENT_REMAP) == 0)
		errExit("Don't Support UFFD_FEATURE_EVENT_REMAP");

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

	/* Write Ops, Trigger #PF */
	*base = 'B';
	printf("UFFD %#lx => %c\n", (unsigned long)base, *base);

	/* REMAP */
	rebase = mremap(base, PAGE_SIZE, PAGE_SIZE * 2,
			MREMAP_MAYMOVE | MREMAP_FIXED, MAP_NEWADDR);
	if (rebase == MAP_FAILED)
		errExit("REMAP");

	/* Write Ops, Triiger #PF */
	*rebase = 'D'; 
	printf("UFFD REMAP %#lx => %c\n", (unsigned long)rebase, *rebase);

	munmap(rebase, PAGE_SIZE * 2);
	close(uffd);

	return 0;
}
