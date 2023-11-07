// SPDX-License-Identifier: GPL-2.0
/*
 * TEMPORARY MAPPING: OSMEM OnDemand 
 *
 * (C) 2023.11.03 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define FILE_PATH		"/dev/BiscuitOS-TEMP"
#define MAP_SIZE		0x1000
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_ONDEMAND	_IO(BISCUITOS_IO, 0x00)

int main()
{
	void *addr;
	int fd;

	/* OPEN DEVICE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", FILE_PATH);
		return -1;
	}

	/* ONLY ALLOC VIRTUAL MEMORY */
	addr = mmap(NULL, MAP_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    0);

	/* OnDemand */
	ioctl(fd, BISCUITOS_ONDEMAND, (unsigned long)addr);

	/* Read from OnDemand Page */
	printf("%#lx => %s\n", (unsigned long)addr, (char *)addr);

	munmap(addr, MAP_SIZE);
	close(fd);

	return 0;
}
