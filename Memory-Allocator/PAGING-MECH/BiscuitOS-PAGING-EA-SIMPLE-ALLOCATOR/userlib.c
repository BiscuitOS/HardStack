// SPDX-License-Identifier: GPL-2.0
/*
 * PAGING Project: Simpler ALLOCATOR
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE       4096
#define ALLOC_FILE	"/dev/BiscuitOS-ALLOC"
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

void *__wrap_malloc(size_t sz)
{
	int fd = open(ALLOC_FILE, O_RDWR);
	void *mem;

	if (fd < 0)
		errExit("ALLOCATOR DOESN'T EXIST.\n");

        mem = mmap(NULL, PAGE_SIZE,
		   PROT_READ | PROT_WRITE,
		   MAP_SHARED,
		   fd,
		   0);
	if (mem == MAP_FAILED) {
		errExit("ALLOCATOR NO FREE MEMORY.\n");
		close(fd);
		return NULL;
	}

	close(fd);
	printf("[ALLOC] %#lx FROM BiscuitOS\n", (unsigned long)mem);

	return mem;
}

void *__wrap_free(void *addr)
{
	munmap(addr, PAGE_SIZE);
	printf("[FREE] %#lx FROM APP.\n", (unsigned long)addr);
}
