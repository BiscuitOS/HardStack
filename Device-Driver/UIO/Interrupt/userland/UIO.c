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
	int ret, info;

	uio_fd = open(UIO_DEV, O_RDWR);
	if (uio_fd < 0) {
		printf("ERROR: open %s\n", UIO_DEV);
		exit(-1);
	}

	/* Enable interrupt */
	info = 1;
	write(uio_fd, &info, sizeof(int));

	while (1) {
		/* listening Interrupt */
		read(uio_fd, &info, sizeof(int));

		/* Disable interrupt */
		info = 0;
		write(uio_fd, &info, sizeof(int));

		/* Handle interrupt on userspace */
		printf("Interrupt from Driver\n");
		sleep(1);

		/* Enable interrupt */
		info = 1;
		write(uio_fd, &info, sizeof(int));
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
