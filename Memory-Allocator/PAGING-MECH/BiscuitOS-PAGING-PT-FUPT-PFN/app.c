// SPDX-License-Identifier: GPL-2.0
/*
 * FOLLOW USERSPACE PGTABLE: Consult RSVDMEM PFN
 *
 *   CMDLINE ADD "memmap=4K$0x10000000"
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
#include <sys/ioctl.h>

#define FILE_PATH	"/dev/BiscuitOS-FUPT"
#define DEV_PATH	"/dev/mem"
#define RSVDMEM_BASE	0x10000000
#define RSVDMEM_SIZE	0x1000
#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

/* TRANS DATA */
struct bs_data {
	unsigned long vaddr;
	unsigned long pfn;
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
	addr = mmap(NULL, RSVDMEM_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED,
		    fd,
		    RSVDMEM_BASE);
	if (addr == MAP_FAILED)
		errExit("mmap failed\n") ;

	/* MAPPING RSVDMEM MEMORY */
	*(char *)addr = 'B';
	printf("%#lx => %c\n", (unsigned long)addr, *(char *)addr);
	sleep(0.2); /* Just for Debug */

	/* OPEN CONSULT INTERFACE */
	fd2 = open(FILE_PATH, O_RDWR);
	if (fd2 < 0)
		errExit("Open '/dev/BiscuitOS-PUPT Failed.\n'");

	/* CONSULT PFN */
	bsdata.vaddr = (unsigned long)addr;
	read(fd2, &bsdata, sizeof(struct bs_data));
	printf("FUPT-PFN %#lx\n", bsdata.pfn);

	/* RECLAIM */
	close(fd2);
	munmap(addr, RSVDMEM_SIZE);
	close(fd);

	return 0;
}
