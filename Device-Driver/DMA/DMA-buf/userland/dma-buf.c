/*
 * DMA-buffer Application
 *
 * (C) 2019.09.24 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

/* IOCTL */
#define GET_DMA_BUF	_IOWR('q', 13, struct dma_info)

struct dma_info {
	uint32_t fd;
	uint32_t size;
	uint32_t phy_addr;
};

int main()
{
	unsigned long *vaddr;
	int fd;
	struct dma_info dinfo;

	/* Open */
	fd = open("/dev/BiscuitOS", O_RDWR, 0644);
	if (fd < 0) {
		printf("Unable to open DMA-buf\n");
		return -1;
	}

	/* memset */
	memset(&dinfo, 0x00, sizeof(struct dma_info));
	
	/* setup dma-buf */
	dinfo.size = 0x100000;

	if (ioctl(fd, GET_DMA_BUF, &dinfo) < 0) {
		printf("DMA-buf alloc fail\n");
		close(fd);
	}
	printf("DMA Physical: %#x\n", dinfo.phy_addr);

	/* remap DMA-buf */
	vaddr = (unsigned long *)mmap(NULL,
				      dinfo.size,
				      PROT_READ | PROT_WRITE,
				      MAP_SHARED,
				      dinfo.fd,
				      0);
	if (vaddr == MAP_FAILED) {
		printf("DMA-Buffer map error!\n");
		close(fd);
		return -1;
	}
	strcpy((char *)vaddr, "BiscuitOS");
	printf("DMA-buf Context: %s\n", vaddr);

	/* Release */
	munmap(vaddr, dinfo.size);
	close(fd);

	return 0;
}
