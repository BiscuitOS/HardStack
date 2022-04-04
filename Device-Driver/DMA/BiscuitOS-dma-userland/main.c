/*
 * BiscuitOS DMA module on Userspace
 *
 * (C) 2020.10.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.04.01 BisuitOS 
 *          <https://biscuitos.github.io/blog/BiscuitOS_Catalogue/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

/* PATH for DMA device */
#define DEV_PATH		"/dev/BiscuitOS-DMA"
/* DMA IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_DMA_FROM_PCI	_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_DMA_TO_PCI	_IO(BISCUITOS_IO, 0x01)
#define BISCUITOS_DMA_TO_USER	_IO(BISCUITOS_IO, 0x02)

struct dma_desc {
	unsigned int dma_base;
	unsigned int pci_base;
	unsigned int size;
	int dma_finish;
	char data[128];
};

int main()
{
	struct dma_desc data;
	int rvl = 0;
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/*
	 * DMA
	 *   Mov Data from DMA
	 *   PCI [0x40000, 0x41000]
	 *   DMA [DMA-addr + 0x0000, DMA-addr + 0x1000] 
	 */
	data = (struct dma_desc) {
		.dma_base = 0x00000,
		.pci_base = 0x40000,
		.size     = 0x1000,
	};
	strcpy(data.data, "Hello BiscuitOS\n");

	rvl = ioctl(fd, BISCUITOS_DMA_TO_PCI, (unsigned long)&data);
	if (fd < 0) {
		printf("ERROR: IOCTL BISCUITOS_DMA_TO_PCI.\n");
		goto out;
	}
	sleep(1); /* wait finish skip interrupt */

	/*
	 * PCI
	 *   Mov Data from PCI Device
	 *   PCI [0x140000, 0x15000]
	 *   DMA [DMA-addr + 0x1000, DMA-addr + 0x2000] 
	 */
	memset(&data, 0x00, sizeof(data));
	data = (struct dma_desc) {
		.dma_base = 0x1000,
		.pci_base = 0x140000,
		.size     = 0x1000,
	};
	rvl = ioctl(fd, BISCUITOS_DMA_FROM_PCI, (unsigned long)&data);
	if (fd < 0) {
		printf("ERROR: IOCTL BISCUITOS_DMA_FROM_PCI.\n");
		goto out;
	}

	/*
	 * Userspace
	 *   Mov Data from DMA to Userspace (wait Interrupt)
	 */
	do {
		memset(&data,  0x00, sizeof(data));
		rvl = ioctl(fd, BISCUITOS_DMA_TO_USER, (unsigned long)&data);
		if (fd < 0) {
			printf("ERROR: IOCTL BISCUITOS_DMA_TO_USER.\n");
			goto out;
		}

		if (data.dma_finish) {
			printf("Userspace-Data: [%s]\n", data.data);
			break;
		} else
			sleep(1);
	} while (1);


out:
	close(fd);
	return rvl;
}
