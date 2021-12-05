/*
 * Hugetlb: SYS V SHMEM Server on shared anonymous hugepage
 *
 * (C) 2021.11.29 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __aarch64__
#error "Process Need running on ARM64 Architecture"
#endif
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

#ifndef SHM_HUGE_64KB
#define HUGETLB_FLAG_ENCODE_SHIFT	26
#define SHM_HUGE_64KB			(16 << HUGETLB_FLAG_ENCODE_SHIFT)
#endif

#define HPAGE_SIZE	(64 * 1024)
#define BISCUITOS_BUFSZ	(2 * HPAGE_SIZE)

int main()
{
	char *shmaddr;
	int shmid;
	int ret;
	key_t key = 2;

	/* Create SHMEM */
	shmid = shmget(key, BISCUITOS_BUFSZ,
				IPC_CREAT | SHM_R | SHM_W |
				SHM_HUGETLB | SHM_HUGE_64KB);
	if (shmid < 0) {
		perror("shmget");
		exit(-1);
	}

	/* SHMEM MMAP */
	shmaddr = shmat(shmid, 0, 0);
	if (shmaddr < 0) {
		perror("shmat");
		exit(-1);
	}

	/* Copy date into SHMEM */
	bzero(shmaddr, BISCUITOS_BUFSZ);
	strcpy(shmaddr, "Hello BiscuitOS on Shared Anonymous Hugepage!\n");

	/* Only for debug */
	sleep(-1);
	return 0;
}
