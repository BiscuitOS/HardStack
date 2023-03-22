// SPDX-License-Identifier: GPL-2.0
/*
 * IOVEC: Copy data from Userspace to kernel Space
 * 
 * (C) 2023.03.21 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/uio.h>

/* PATH for device */
#define DEV_PATH		"/dev/BiscuitOS-IOV"
/* Data */
static char *astr = "Hello ";
static char *bstr = "BiscuitOS";

int main()
{
	struct iovec iov[2];
	int fd;

	/* open device */
	fd = open(DEV_PATH, O_RDWR);
	if (fd < 0) {
		printf("ERROR: Can't open %s\n", DEV_PATH);
		return -1;
	}

	iov[0].iov_base = astr;
	iov[0].iov_len	= strlen(astr);
	iov[1].iov_base = bstr;
	iov[1].iov_len	= strlen(bstr);
	if (writev(fd, iov, 2) < 0) {
		printf("WRITEV ERROR.!\n");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}
