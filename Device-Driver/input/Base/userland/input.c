/*
 * Input Device Driver Application
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

#define INPUT_SYS_PATH	"/sys/devices/platform/Input_demo.1/input/"

int main()
{
	struct dirent *ptr;
	char events[128];
	char *input_dir = NULL;
	DIR *dir;

	if ((dir = opendir(INPUT_SYS_PATH)) == NULL) {
		printf("Open %s dirent error.\n", INPUT_SYS_PATH);
		return -1;
	}

	while ((ptr = readdir(dir)) != NULL) {
		if (strcmp(ptr->d_name, ".") == 0 ||
		    strcmp(ptr->d_name, "..") == 0) {
			continue;
		}
		/* Search INPUT_SYS_PATH/input direct */
		if (ptr->d_type == DT_DIR) {
			if (!strncmp(ptr->d_name, "input", strlen("input"))) {
				/* Find special input direct */
				input_dir=ptr->d_name;
				/* Create full path name */
				strcpy(events, INPUT_SYS_PATH);
				strcat(events, "/");
				strcat(events, input_dir);
				printf("FULL: %s\n", events);
			}
		}
	}

	/* close */
	closedir(dir);

	return 0;
}
