/*
 * UIO Userland
 *
 * (C) 2019.10.24 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

#define UIO_DEV		"/dev/uio0"
#define UIO_ADDR	"/sys/class/uio/uio0/maps/map0/addr"
#define UIO_SIZE	"/sys/class/uio/uio0/maps/map0/size"

static char uio_addr_buf[32];
static char uio_size_buf[32];

int main()
{
	void *uio_addr, *access_address;
	int fd_uio, fd_addr, fd_size;
	int uio_size;
	int ret;

	/* Open special file */
	fd_uio = open(UIO_DEV, O_RDWR);
	if (fd_uio < 0) {
		printf("ERROR: open %s\n", UIO_DEV);
		ret = fd_uio;
		goto err_dev;
	}
	fd_addr = open(UIO_ADDR, O_RDONLY);
	if (fd_addr < 0) {
		printf("ERROR: open %s\n", UIO_ADDR);
		ret = fd_addr;
		goto err_addr;
	}
	fd_size = open(UIO_SIZE, O_RDONLY);
	if (fd_size < 0) {
		printf("ERROR: open %s\n", UIO_SIZE);
		ret = fd_size;
		goto err_size;
	}
	
	/* Read */
	read(fd_addr, uio_addr_buf, sizeof(uio_addr_buf));
	read(fd_size, uio_size_buf, sizeof(uio_size_buf));

	uio_addr = (void *)strtoul(uio_addr_buf, NULL, 0);
	uio_size = (int)strtol(uio_size_buf, NULL, 0);

	access_address = mmap(NULL, uio_size, PROT_READ | PROT_WRITE,
				MAP_SHARED, fd_uio, 0);
	if (access_address == (void *)-1) {
		printf("ERROR: mmap uio\n");
		goto err_map;
	}

	printf("Device Address %p (length: %d) virutal address: %p\n",
			uio_addr, uio_size, access_address);

	/* unmap */
	munmap(access_address, uio_size);

	close(fd_size);
	close(fd_addr);
	close(fd_uio);

	return 0;

err_map:
err_size:
	close(fd_size);
err_addr:
	close(fd_uio);
err_dev:
	return ret;
}
