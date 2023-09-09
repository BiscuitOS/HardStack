// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault ERROR CODE: PF_INSTR
 *
 * (C) 2023.09.03 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE	4096
typedef int (*BiscuitOS_func_t)(void);

int main()
{
	void *base;
	int fd;

	fd = open("/usr/bin/BIN.bin", O_RDONLY);
	if (fd < 0) {
		printf("Open BIN.bin failed.\n");
		exit(-1);
	}

	base = mmap(NULL, MAP_SIZE,
		    PROT_READ | PROT_EXEC,
		    MAP_PRIVATE,
		    fd, 0);
	if (base == MAP_FAILED) {
		printf("MMAP FAILED.\n");
		close(fd);
		exit(-1);
	}
	
	BiscuitOS_func_t func = (unsigned long)base + 0x10;
	printf("INSTR VADDR %#lx\n", (unsigned long)base);

	/* Execute Case #PF with PF_INSTR */
	func();

	munmap(base, MAP_SIZE);
	close(fd);

	return 0;
}
