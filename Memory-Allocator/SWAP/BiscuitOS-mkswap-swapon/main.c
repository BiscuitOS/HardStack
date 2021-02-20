/*
 * mkswap and swapon on BiscuitOS
 *
 * (C) 2021.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>

#define SWAP_PATH		"/BiscuitOS_swap"
#define PAGE_SIZE		(4096)
#define ALIGN(m)		__attribute__ ((__aligned__(m)))

#define __NR_SWAPON		87

/*
 * Magic header for a swap area. ... Note that the first
 * kilobyte is reserved for boot loader or disk label stuff.
 */
struct swap_header_v1 {
/*      char     bootbits[1024];    Space for disklabel etc. */
	uint32_t version;        /* second kbyte, word 0 */
	uint32_t last_page;      /* 1 */
	uint32_t nr_badpages;    /* 2 */
	char     sws_uuid[16];   /* 3,4,5,6 */
	char     sws_volume[16]; /* 7,8,9,10 */
	uint32_t padding[117];   /* 11..127 */
	uint32_t badpages[1];    /* 128 */
	/* total 129 32-bit words in 2nd kilobyte */
} __attribute__((__may_alias__));

/* Zero Buffer */
static char zero_buffer[1024] ALIGN(sizeof(long long));
#define hdr	((struct swap_header_v1*)zero_buffer)

/* SWAP SIG */
static const char SWAPSPACE2[sizeof("SWAPSPACE2")] ALIGN(1) = "SWAPSPACE2";

int main()
{
	int fd;

	fd = open(SWAP_PATH, O_WRONLY, 0666);
	if (fd < 0) {
		printf("Swap file doesn't exist: %s\n", SWAP_PATH);
		return -1;
	}

	memset(hdr, 0, 1024);
	/* hdr is zero-filled so far. Clear the first kbyte, or else
	 * mkswap-ing former FAT partition does NOT erase its signature.
	 *
	 * util-linux-ng 2.17.2 claims to erase it only if it does not see
	 * a partition table and is not run on whole disk. -f forces it.
	 */
	write(fd, hdr, 1024);

	/* Fill the header. */
	hdr->version = 1;
	hdr->last_page = 0;
	/* UUID: Emulate a UUID */
	memset(&hdr->sws_uuid[0],  0x65862b10, 4);
	memset(&hdr->sws_uuid[4],  0xa3457fc3, 4);
	memset(&hdr->sws_uuid[8],  0xe7c1ad90, 4);
	memset(&hdr->sws_uuid[12], 0x55897cd4, 4);
	strncpy(hdr->sws_volume, SWAP_PATH, 16);

	/* Write the header.  Sync to disk because some kernel versions check
	 * signature on disk (not in cache) during swapon. */
	write(fd, hdr, 516);
	lseek(fd, PAGE_SIZE - 10, SEEK_SET);
	write(fd, SWAPSPACE2, 10);
	fsync(fd);
	close(fd);

	syscall(600, 1);
	syscall(__NR_SWAPON, SWAP_PATH, 0);
	syscall(600, 0);

	printf("Hello BiscuitOS.\n");
	return 0;
}
