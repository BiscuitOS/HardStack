// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: VMA VM_UFFD_MISSING
 *
 * (C) 2023.12.20 BuddyZhang1 <buddy.zhang@aliyun.com>
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
	struct uffdio_copy uffdio_copy;
	struct uffd_msg msg;
	char *ondemand_page;
	long uffd = (long)arg;

	/* On-Demand Memory */
	ondemand_page = malloc(PAGE_SIZE);
	if (!ondemand_page)
		errExit("OnDemand Page.");

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

		/* Expect only EVENT PAGEFAULT */
		if (msg.event != UFFD_EVENT_PAGEFAULT)
			errExit("Unexpected event on userfaultfd");

		/* Display info about #PF event */
		printf("<UFFD> EVENT:   UFFD_EVENT_PAGEFAULT\n");
		printf("<UFFD> FLAGS:   %#llx\n", msg.arg.pagefault.flags);
		printf("<UFFD> ADDRESS: %#llx\n", msg.arg.pagefault.address);

		/* On-Demand Alloc Memory */
		sprintf(ondemand_page, "Hello BiscuitOS");

		uffdio_copy.src = (unsigned long)ondemand_page;
		uffdio_copy.dst = (unsigned long)msg.arg.pagefault.address &
						 ~(PAGE_SIZE - 1);
		uffdio_copy.len = PAGE_SIZE;
		uffdio_copy.mode = 0;
		uffdio_copy.copy = 0;
		if (ioctl(uffd, UFFDIO_COPY, &uffdio_copy) == -1)
			errExit("IOCTL: UFFDIO_COPY");
		printf("<UFFD> RETURN   %#llx\n", uffdio_copy.copy);
	}
}

int main()
{
	struct uffdio_register uffdio_register;
	struct uffdio_api uffdio_api;
	char *base;
	pthread_t thr;
	long uffd;

	/* Create and Enable userfaultfd object */
	uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
	if (uffd < 0)
		errExit("Userfaultfd create failed.");

	uffdio_api.api = UFFD_API;
	uffdio_api.features = 0;
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
	uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;
	if (ioctl(uffd, UFFDIO_REGISTER, &uffdio_register) == -1)
		errExit("IOCTL: UFFDIO_REGISTER");

	/* Create a thread that will process the userfaultfd events */
	if (pthread_create(&thr, NULL, fault_handler_thread, (void *)uffd) != 0)
		errExit("Pthread Create.\n");

	/* Read Ops, Trigger #PF */
	*base = 'B'; /* MARK VM_UFFD_MISSING */
	printf("UFFD %#lx => %c\n", (unsigned long)base, *base);

	munmap(base, PAGE_SIZE);
	close(uffd);

	return 0;
}
