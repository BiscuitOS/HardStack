// SPDX-License-Identifier: GPL-2.0
/*
 * Protection Key
 *
 * (C) 2023.11.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE		(4096)
#define MAP_VADDR		(0x6000000000)
#define PKEY_DISABLE_ACCESS	0x1
#define PKEY_DISABLE_WRITE	0x2	
#define SYS_pkey_alloc		330
#define SYS_mprotect_key	329
#define SYS_pkey_free		331

static inline int wrpkru(unsigned int pkru)
{
	unsigned long eax = pkru;
	unsigned long ecx = 0;
	unsigned long edx = 0;

	asm volatile (".byte 0x0f,0x01,0xef\n\t"
		     :: "a" (eax), "c" (ecx), "d" (edx));
	return 0;
}

static int pkey_set(int pkey, unsigned long right, unsigned long flags)
{
	unsigned int pkru = (right << (2 * pkey));

	return wrpkru(pkru);
}

int main()
{
	int pkey, status;
	char *base, ch;

	base = mmap((void *)MAP_VADDR, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS,
		    -1,
		    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		return -1;
	}

	/* Write Ops, Trigger #PF with PF_PROT */
	*base = 'B';
	/* Read Ops, Don't Trigger #PF */
	printf("PK %#lx => %c\n", (unsigned long)base, *base);
	/* Just debug */
	sleep(1);

	/* Allocate a protection key */
	pkey = syscall(SYS_pkey_alloc, 0, 0);
	if (pkey < 0)
		printf("PKEY Alloc Failed.\n");

	/*
	 * Disable access to any memory with 'pkey' set.
	 * event though there is none right now.
	 */
	status = pkey_set(pkey, PKEY_DISABLE_ACCESS, 0);
	if (status < 0)
		printf("PKEY SET Failed.\n");

	/*
	 * Set the protection key on "base".
	 * Note that it is still read/write as far as mprotect() is
	 * concerned and the previous pkey_set() overrides it.
	 */
	status = syscall(SYS_mprotect_key, base, PAGE_SIZE,
				PROT_READ | PROT_WRITE, pkey);
	if (status < 0)
		printf("PKEY MPROTECT Failed.\n");

	/* 
	 * CRASH Here, becase we have disallowed access.
	 * Trigger #PF with PF_PK
	 */
	ch = *base;

	/* Pkey finish */
	status = syscall(SYS_pkey_free, pkey);
	if (status < 0)
		printf("PKEY Free Failed.\n");

	munmap(base, PAGE_SIZE);

	return 0;
}
