// SPDX-License-Identifier: GPL-2.0
/*
 * CLEAR_REFS: SoftDirty
 *
 * (C) 2023.08.29 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>

#define PAGE_SIZE	(4096)
#define MAP_VADDR	(0x6000000000)
#define PM_SOFT_DIRTY	55
#define PM_PRESENT	63

static int detect_soft_dirty(int pid, unsigned long vaddr)
{
	int pagesize = getpagesize();
	unsigned long vpage_index;
	unsigned long pfn, pgoff;
	unsigned long voffset;
	char buffer[64];
	uint64_t item = 0;
	int fd;

	vpage_index = vaddr / pagesize;
	voffset = vpage_index * sizeof(uint64_t);
	pgoff = vaddr % pagesize;

	/* open pagemap */
	sprintf(buffer, "/proc/%d/pagemap", pid);
	fd = open(buffer, O_RDONLY);
	if (fd < 0) {
		printf("Open %s failed.\n", buffer);
		return 0;
	}

	if (lseek(fd, voffset, SEEK_SET) < 0) {
		printf("ERROR: lseek failed.\n");
		close(fd);
		return 0;
	}

	if (read(fd, &item, sizeof(uint64_t)) != sizeof(uint64_t)) {
		printf("ERROR: read item error.\n");
		close(fd);
		return 0;
	}

	if ((((uint64_t)1 << PM_PRESENT) & item) == 0) {
		close(fd);
		return 0;
	}

	close(fd);
	if (item & ((uint64_t)1 << PM_SOFT_DIRTY))
		return 1;
	else
		return 0;
}

int main(int argc, char *argv[])
{
	unsigned long vaddr;
	int soft_dirty;
	int pid;

	if (argc < 3) {
		printf("ERROR: %s PID VADDR\n", argv[0]);
		exit(-1);
	}

	sscanf(argv[1], "%d", &pid);
	sscanf(argv[2], "%lx", &vaddr);

	soft_dirty = detect_soft_dirty(pid, vaddr);
	printf("PID-%d: Vaddr %#lx Soft-dirty: %d\n",
					pid, vaddr, soft_dirty);

	return 0;
}
