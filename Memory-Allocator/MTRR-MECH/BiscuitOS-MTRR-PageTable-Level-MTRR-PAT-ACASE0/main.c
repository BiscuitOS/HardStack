/*
 * BiscuitOS Paing Mechanism TimeCost Application
 *
 * UC(2): WB + UC  --> 8124597 nsec 
 * UC(2): WB + UC- --> 8114272 nsec
 * WC:    WB + WC  --> 8070646 nsec 
 * WT:    WB + WT  --> 8204126 nsec
 * WB:    WB + WB  --> 8349259 nsec 
 * WP:    WB + WP  --> 8081591 nsec
 *
 * (C) 2020.10.06 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
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
#include <sys/types.h>
#include <sys/mman.h>

#define DEV_PATH		"/dev/BiscuitOS"
#define MMAP_SIZE		(4 * 1024)
#define REPLAY_COUNT		(1024 * 1024 / 4) /* 1Gig */

int main()
{
	unsigned long count = REPLAY_COUNT;
	unsigned long tv_ms, ntv_ms;
	struct timeval tv, ntv;
	void *base;
	char *val;
	int fd, i;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	/* mmap */
	base = mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE,
			  MAP_PRIVATE | MAP_POPULATE, fd, 0);
	if (base == MAP_FAILED) {
		printf("ERROR: mmaping failed.\n");
		close(fd);
		return -1;
	}
	val = (char *)base;
	
	gettimeofday(&tv, NULL);
	/* usage */
	while (count--)
		for (i = 0; i < MMAP_SIZE; i++) {
			char data; 

			/* READ */
			data = val[i];

			/* WRITE */
			val[i] = i;
		}
	gettimeofday(&ntv, NULL);
	printf("Cost Time: %ld nsec\n", ntv.tv_sec * 1000000 + ntv.tv_usec -
					 tv.tv_sec * 1000000 - tv.tv_usec);

	munmap(base, MMAP_SIZE);

	close(fd);
	return 0;
}
