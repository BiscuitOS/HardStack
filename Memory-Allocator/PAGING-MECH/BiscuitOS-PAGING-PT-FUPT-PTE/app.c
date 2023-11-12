// SPDX-License-Identifier: GPL-2.0
/*
 * FOLLOW USERSPACE PGTABLE: Consult PTE
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

#define FILE_PATH		"/dev/BiscuitOS-FUPT"
/* TRANS DATA */
struct bs_data {
	unsigned long vaddr;
	unsigned long pte_val;
} bsdata;

int main()
{
	void *addr;
	int fd;

	/* OPEN INTERFACE */
	fd = open(FILE_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: open %s failed.\n", FILE_PATH);
		return -1;
	}

	/* ALLOC MEMORY  */
	addr = malloc(64);

	/* MAPPING PHYSICAL MEMORY */
	*(char *)addr = 'B';
	printf("%#lx => %c\n", (unsigned long)addr, *(char *)addr);
	sleep(1); /* Just for Debug */

	/* CONSULT PTE */
	bsdata.vaddr = (unsigned long)addr;
	read(fd, &bsdata, sizeof(struct bs_data));
	printf("FUPT-PTE %#lx\n", bsdata.pte_val);

	/* RECLAIM */
	free(addr);
	close(fd);

	return 0;
}
