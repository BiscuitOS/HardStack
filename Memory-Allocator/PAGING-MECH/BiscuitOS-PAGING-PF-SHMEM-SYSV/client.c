// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault with Shmem Memory on SYSV
 *
 * (C) 2023.09.22 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAP_SIZE	(4096)
#define ADDR		(void *)(0x6000000000UL)
#define SHMAT_FLAGS	(SHM_RND)
#define SHMEM_KEY	((key_t)0x88520)

int main()
{
	char *base;
	int shmid;

	/* Display SHMEM information */
	system("ipcs -m");

	/* Create SHMEM */
	shmid = shmget(SHMEM_KEY, MAP_SIZE, IPC_CREAT | SHM_R | SHM_W);
	if (shmid < 0) {
		perror("shmget");
		exit(-1);
	}

	/* SHMEM MMAP */
	base = shmat(shmid, ADDR, SHMAT_FLAGS);
	if (base < 0) {
		perror("shmat");
		exit(-1);
	}

	/* Write Ops, Trigger #PF */
	*base = 'H';

	/* Read data from SHMEM */
	printf("Anonymous: %s\n", base);

	/* Separate from current processor */
	if (shmdt(base) < 0 ) {
		perror("shmdt");
		exit(1);
	}

	/* Delete SHMEM */
	shmctl(shmid, IPC_RMID, NULL);

	return 0;
}
