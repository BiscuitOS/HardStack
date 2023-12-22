// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: VMA VM_EXEC
 *
 * (C) 2023.12.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	4096
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)
typedef int (*BiscuitOS_func_t)(void);

int main()
{
	void *base;
	int fd;

	/* OPEN EXEC FILE */
	fd = open("/usr/bin/BIN.bin", O_RDONLY);
	if (fd < 0)
		errExit("OPEN FAILED\n");

	/* ALLOC EXEC MEMORY */
	base = mmap(NULL, MAP_SIZE,
		    PROT_READ | PROT_EXEC, /* PROT_EXEC TO VM_EXEC */
		    MAP_PRIVATE,
		    fd, 0);
	if (base == MAP_FAILED)
		errExit("MMAP FAILED.\n");
	
	/* SETUP EXEC CODE */
	BiscuitOS_func_t func = (unsigned long)base + 0x10;
	printf("INSTR VADDR %#lx\n", (unsigned long)base);

	/* Execute Case #PF with PF_INSTR */
	func();

	/* RECLAIM */
	munmap(base, MAP_SIZE);
	close(fd);

	return 0;
}
