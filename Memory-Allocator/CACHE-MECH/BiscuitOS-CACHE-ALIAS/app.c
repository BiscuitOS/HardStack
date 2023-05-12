// SPDX-License-Identifier: GPL-2.0
/*
 * CACHE Alias
 *
 * (C) 2020.10.06 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define DEV_PATH		"/dev/BiscuitOS"
#define PAGE_SIZE		4096
#define BISCUITOS_IO		0xAE
#define BISCUITOS_ONDEMAND	_IO(BISCUITOS_IO, 0x00)

int main()
{
	void *addr;
	int fd, r;

	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* Only Alloc Virtual Memory */
	addr = mmap(NULL, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_ANONYMOUS | MAP_SHARED,
		    -1, 0);
	if (addr == MAP_FAILED) {
		printf("ERROR: Memory In-Short.\n");
		r = -1;
		goto err_mmap;
	}

	/* On-Demand Alloc Memory  */
	r = ioctl(fd, BISCUITOS_ONDEMAND, (unsigned long)addr);
	if (fd < 0) {
		printf("ERROR: Sysytem can handle IOCTL.\n");
		r = -1;
		goto err_ioctl;
	}

	/* No trigger Page-Fault */
	printf("USER-ADDR %#lx: %s\n", (unsigned long)addr, (char *)addr);

	r = 0; /* Normal exit */

err_ioctl:
	munmap(addr, PAGE_SIZE);
err_mmap:
	close(fd);
	return r;
}
