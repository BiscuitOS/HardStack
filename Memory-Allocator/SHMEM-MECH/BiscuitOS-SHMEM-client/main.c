/*
 * SHMEM: Client
 *
 * (C) 2021.08.02 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define BISCUITOS_BUFSZ	2048

int main()
{
	char *shmaddr;
	int shmid;
	int ret;
	key_t key;

	/* Create Key */
	key = ftok("../", 2048);
	if (key == -1)
		perror("ftok");

	/* Display SHMEM information */
	system("ipcs -m");

	/* Create SHMEM */
	shmid = shmget(key, BISCUITOS_BUFSZ, IPC_CREAT | 0666);
	if (shmid < 0) {
		perror("shmget");
		exit(-1);
	}

	/* SHMEM MMAP */
	shmaddr = shmat(shmid, NULL, 0);
	if (shmaddr < 0) {
		perror("shmat");
		exit(-1);
	}

	/* Read data from SHMEM */
	printf("Ezekiel: %s\n", shmaddr);

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
