// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: MAP_STACK
 *
 * (C) 2023.12.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>

#define MAP_SIZE	(4096)
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

/* SIGNAL HANDLER */
static void signal_handler(int signum)
{
	printf("Receive SIGNAL %d on Alternate STACK\n", signum);
}

int main()
{
	struct sigaction sa;
	stack_t stk;
	void *mem;

	/* LAZYALLOC ANONYMOUS MEMORY */
	mem = mmap(NULL, MAP_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
		   -1,
		   0);
	if (mem == MAP_FAILED)
		errExit("MMAP FAILED.\n");
	stk.ss_size = SIGSTKSZ;
	stk.ss_flags = 0;
	printf("STACK ADDRESS: %#lx\n", (unsigned long)mem);

	/* REGISTER SIGNAL */
	if (sigaltstack(NULL, &stk) == -1)
		errExit("REGISTER SIGNAL\n");

	/* SETUP SIGNAL HANDLER */
	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_ONSTACK; /* USER ALTERNATE STACK */
	if (sigaction(SIGUSR1, &sa, NULL) == -1)
		errExit("SIGACTION FAILED.\n");

	printf("Sending signal %d\n", SIGUSR1);
	raise(SIGUSR1);

	/* RECLAIM */
	munmap(mem, MAP_SIZE);

	return 0;
}
