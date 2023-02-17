/*
 * Alloc Memory From VMALLOC on USERSPACE
 *
 * (C) 2023.02.18 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* BiscuitOS VMALLOC syscall */
#define __NR_BISCUITOS_VMALLOC_ALLOC	601
#define __NR_BISCUITOS_VMALLOC_FREE	602

#define PAGE_SIZE			4096

static inline void * BiscuitOS_vmalloc_alloc(unsigned long size)
{
	return (void *)syscall(__NR_BISCUITOS_VMALLOC_ALLOC);
}

static inline void BiscuitOS_vmalloc_free(void *addr)
{
	syscall(__NR_BISCUITOS_VMALLOC_FREE, addr);
}

int main()
{
	void *mem;

	/* alloc */
	mem = BiscuitOS_vmalloc_alloc(PAGE_SIZE);
	if (!mem) {
		printf("ERROR: no free memory from VMALLOC.\n");
		return -ENOMEM;
	}

	/* use */
	sprintf((char *)mem, "Hello BiscuitOS");
	printf("%s: %#lx\n", (char *)mem, (unsigned long)mem);

	/* free */
	BiscuitOS_vmalloc_free(mem);
	mem = NULL;

	return 0;
}
