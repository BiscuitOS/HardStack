// SPDX-License-Identifier: GPL-2.0
/*
 * Alloc Specify Alignment virtual Address
 *
 * (C) 2023.05.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MEMORY_SIZE		4096
#define VA_ALIGN(size, ps) (((size) + ((ps) - 1)) & ~((ps) - 1))
#define ALIGN_SIZE_4K		(4 * 1024)
#define ALIGN_SIZE_2M		(2 * 1024 * 1024)
#define ALIGN_SIZE_1M		(1 * 1024 * 1024)
#define ALIGN_SIZE_1G		(1 * 1024 * 1024 * 1024)

/*
 * Alloc alignment virtual address
 *
 *  @align_size: size for alignment
 */
static void *alignment_vaddr(unsigned long align_size,
		size_t size, int prot, int flags, int fd, __off_t offset)
{
	unsigned long total_size = size + 2 * align_size;
	void *addr, *target_addr;

	addr =  mmap(NULL, total_size, prot, flags, fd, offset);
	if (addr == MAP_FAILED)
		return addr;

	target_addr = VA_ALIGN((unsigned long)addr, align_size);
	total_size -= (unsigned long)target_addr - (unsigned long)addr + size;
	munmap(addr, (unsigned long)target_addr - (unsigned long)addr);
	munmap((char *)target_addr + size, total_size);

	return target_addr;
}

int main()
{
	void *addr;

	addr = alignment_vaddr(
		    ALIGN_SIZE_1M, 
		    MEMORY_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS,
		    -1,
		    0);
	if (addr == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -ENOMEM;
	}

	/* Trigger page fault */
	*(char *)addr = 'B';
	printf("%#lx => %c\n", (unsigned long)addr, *(char *)addr);

	munmap(addr, MEMORY_SIZE);

	return 0;
}
