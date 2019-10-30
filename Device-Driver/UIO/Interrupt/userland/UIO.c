/*
 * UIO Interrupt Userland
 *
 * (C) 2019.10.30 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>

#define UIO_DEV		"/dev/uio0"

int do_thread(void)
{
	int uio_fd;
	int ret, c;

	uio_fd = open(UIO_DEV, O_RDWR);
	if (uio_fd < 0) {
		printf("ERROR: open %s\n", UIO_DEV);
		exit(-1);
	}

	/* listen interrupt */
	while (1) {
		ret = read(uio_fd, &c, sizeof(int));
		c = 1;
		printf("Interrupt from Driver\n");
		write(uio_fd, &c, sizeof(int));
	}

	close(uio_fd);

	return 0;
}

int main()
{
	pthread_t int_thread;
	int ret;

	ret = pthread_create(&int_thread, NULL, (void *)do_thread, NULL);
	if (ret < 0) {
		printf("ERROR: Create pthread\n");
		return -1;
	}

	while (1);

	return 0;
}
