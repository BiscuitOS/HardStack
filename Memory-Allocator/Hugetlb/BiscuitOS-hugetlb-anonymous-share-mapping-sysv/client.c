/*
 * Hugetlb: SYS V SHMEM Client on Shared Anonymous Hugepage!
 *
 * (C) 2021.11.29 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#ifndef SHM_HUGETLB
#define SHM_HUGETLB 04000
#endif

/* Only ia64 requires this */
#ifdef __ia64__
#define ADDR (void *)(0x8000000000000000UL)
#define SHMAT_FLAGS (SHM_RND)
#else
#define ADDR (void *)(0x0UL)
#define SHMAT_FLAGS (0)
#endif

#define HPAGE_SIZE	(2UL * 1024 * 1024)
#define BISCUITOS_BUFSZ	(2 * HPAGE_SIZE)

int main()
{
	char *shmaddr;
	int shmid;
	int ret;
	key_t key = 2;

	/* Display SHMEM information */
	system("ipcs -m");

	/* Create SHMEM */
	shmid = shmget(key, BISCUITOS_BUFSZ, SHM_HUGETLB | IPC_CREAT | SHM_R | SHM_W);
	if (shmid < 0) {
		perror("shmget");
		exit(-1);
	}

	/* SHMEM MMAP */
	shmaddr = shmat(shmid, ADDR, SHMAT_FLAGS);
	if (shmaddr < 0) {
		perror("shmat");
		exit(-1);
	}

	/* Read data from SHMEM */
	printf("Anonymous: %s\n", shmaddr);

	/* Separate from current processor */
	ret = shmdt(shmaddr);
	if (ret < 0) {
		perror("shmdt");
		exit(1);
	}

	/* Delete SHMEM */
	shmctl(shmid, IPC_RMID, NULL);

	/* Display SHMEM information */
	system("ipcs -m");

	return 0;
}
