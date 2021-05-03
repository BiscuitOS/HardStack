/*
 * SYS_mmap: READ_IMPLIES_EXEC
 *
 * (C) 2021.04.02 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <fcntl.h>
#include <sys/mman.h>

#define BISCUITOS_FILE_PATH	"/BiscuitOS-tmpfs/BiscuitOS"
#define BISCUITOS_MAP_SIZE	4096

/* EXEC */
char BiscuitOS_shell[] =
	"\x55"		/* push	%rbp */
	"\x48\x89\xe5"	/* mov	%rsp, %rbp */
	"\x31\xc0"	/* xor	%eax, %eax */
	"\x5d"		/* pop	%rbp */
	"\xc3";		/* retq */

typedef void (*BiscuitOS_func_t)(void);

int main()
{
	BiscuitOS_func_t func;
	char *base;
	int fd;

	/* Open */
	fd = open(BISCUITOS_FILE_PATH, O_RDWR | O_CREAT);
	if (fd < 0) {
		printf("ERROR: Open %s failed.\n", BISCUITOS_FILE_PATH);
		return -EBUSY;
	}

	/* Copy shellcode into file */
	write(fd, BiscuitOS_shell, strlen(BiscuitOS_shell));

	/* mmap */
	base = (char *)mmap(NULL, 
			    BISCUITOS_MAP_SIZE,
			    PROT_READ | PROT_WRITE | PROT_EXEC,
			    MAP_SHARED,
			    fd,
			    0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmap failed.\n");
		close(fd);
		return -ENOMEM;
	}

	/* EXEC: Need execute permission */
	func = (BiscuitOS_func_t)base;
	(void)(*func)();

	/* unmap */
	munmap(base, BISCUITOS_MAP_SIZE);
	close(fd);
	return 0;
}
