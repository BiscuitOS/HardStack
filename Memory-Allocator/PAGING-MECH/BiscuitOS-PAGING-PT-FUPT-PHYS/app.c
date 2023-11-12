// SPDX-License-Identifier: GPL-2.0
/*
 * FOLLOW USERSPACE PGTABLE: Consult MMIO
 *
 * (C) 2023.09.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FILE_PATH	"/dev/BiscuitOS-FUPT"
#define DEV_PATH	"/dev/mem"
#define MMIO_BASE	0xF0000000UL
#define MMIO_SIZE	0x1000UL
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

/* TRANS DATA */
struct bs_data {
	unsigned long vaddr;
	unsigned long phys;
	unsigned long prot;
} bsdata;

int main()
{
	void *addr;
	int fd, fd2;

	/* OPEN DEVMAP INTERFACE */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0)
		errExit("Open '/dev/mem' Failed");

	/* MAPPING SPEICAL RSVDMEM */
	addr = mmap(NULL, MMIO_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    MMIO_BASE);
	if (addr == MAP_FAILED)
		errExit("mmap failed\n") ;

	/* MAPPING MMIO */
	*(char *)addr = 'B';
	printf("%#lx => %c\n", (unsigned long)addr, *(char *)addr);
	sleep(0.2); /* Just for Debug */

	/* OPEN CONSULT INTERFACE */
	fd2 = open(FILE_PATH, O_RDWR);
	if (fd2 < 0)
		errExit("Open '/dev/BiscuitOS-PUPT Failed.\n'");

	/* CONSULT MMIO */
	bsdata.vaddr = (unsigned long)addr;
	read(fd2, &bsdata, sizeof(struct bs_data));
	printf("FUPT-PHYS %#lx PROT %#lx\n", bsdata.phys, bsdata.prot);

	/* RECLAIM */
	close(fd2);
	munmap(addr, MMIO_SIZE);
	close(fd);

	return 0;
}
