// SPDX-License-Identifier: GPL-2.0
/*
 * FLUSH CACHE on DMA-Pool
 *
 * (C) 2023.04.03 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>

/* PATH for device */
#define DEV_PATH		"/dev/BiscuitOS-DMA-Pool"
#define PAGE_SIZE		(4 * 1024)
/* IOCTL CMD */
#define BISCUITOS_IO			0xAE
#define BISCUITOS_DMA_FROM_DEVICE	_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_DMA_TO_DEVICE		_IO(BISCUITOS_IO, 0x01)

int main()
{
	void *data;
	int fd, r;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	data = mmap(NULL, PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd, 0);
	if (data == MAP_FAILED) {
		printf("ERROR: %s map failed\n", DEV_PATH);
		close(fd);
		return -1;
	}

	/* DMA FROM DEVICE */
	ioctl(fd, BISCUITOS_DMA_FROM_DEVICE, 0);
	printf("[1] DMA Context: %s\n", (char *)data);

	/* DMA TO DEVICE */
	sprintf((char *)data, "Hello BiscuitOS on DMA-Pool");
	ioctl(fd, BISCUITOS_DMA_TO_DEVICE, 0);

	/* DMA FROM DEVICE */
	memset((char *)data, 0x00, PAGE_SIZE);
	ioctl(fd, BISCUITOS_DMA_FROM_DEVICE, 0);
	printf("[2] DMA Context: %s\n", (char *)data);

	munmap(data, PAGE_SIZE);
	r = 0; /* Normal exit */
	
out:
	close(fd);
	return r;
}
