/*
 * Platform Device
 *
 * (C) 2019.09.24 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define PLATFORM_PATH "/sys/bus/platform/devices/Platform_Resource/"

int main()
{
	DIR *dir;
	struct dirent *ptr;

	if ((dir = opendir(PLATFORM_PATH)) == NULL) {
		printf("Open %s dirent error.\n", PLATFORM_PATH);
		return -1;
	}

	while ((ptr = readdir(dir)) != NULL) {
		if (strcmp(ptr->d_name, ".") == 0 ||
		    strcmp(ptr->d_name, "..") == 0) {
			continue;
		}
		switch (ptr->d_type) {
		case DT_REG: /* A regular file (DT_REG: 8) */
			printf("[FILE] %s\n", ptr->d_name);
			break;
		case DT_DIR: /* A directory (DT_DIR: 4) */
			printf("[DIRT] %s\n", ptr->d_name);
			break;
		case DT_FIFO: /* A named pipe, or FIFO (DT_FIFO: 1) */
			printf("[FIFO] %s\n", ptr->d_name);
			break;
		case DT_SOCK: /* A local-domain socket (DT_SOCK: 12) */
			printf("[SOCK] %s\n", ptr->d_name);
			break;
		case DT_CHR: /* A character device (DT_CHR: 2) */
			printf("[CHAR] %s\n", ptr->d_name);
			break;
		case DT_BLK: /* A block device (DT_BLK: 6) */
			printf("[BLCK] %s\n", ptr->d_name);
			break;
		case DT_LNK: /* A symbol link (DT_LNK: 10) */
			printf("[LINK] %s\n", ptr->d_name);
			break;
		}
	}

	/* close */
	closedir(dir);

	return 0;
}
