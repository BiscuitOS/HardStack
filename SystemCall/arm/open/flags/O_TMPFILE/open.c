/*
 * open: O_TMPFILE
 *
 * (C) 2019.12.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

int main()
{
	char buffer[20] = "BiscuitOS";
	int fd, ret;

	/* open: Tmpfile, path must a directory */
	fd = open("/etc", O_RDWR | __O_TMPFILE, 
					S_IRUSR | S_IWUSR);
	if (fd < 0) {
		perror("open");
		return -1;
	}
	/* write operation */
	ret = write(fd, buffer, 9);
	if (ret < 0) {
		perror("Write failed");
		close(fd);
		return -1;
	}

	/* set on start */
	lseek(fd, 0, SEEK_SET);

	/* Force read with O_WRONLY */
	memset(buffer, 0, 20);
	ret = read(fd, buffer, 10);
	if (ret < 0) {
		perror("Read failed");
		close(fd);
		return -1;
	}
	buffer[10] = '\0';
	printf("Read contents: %s\n", buffer);

	/* close */
	close(fd);

	return 0;
}
