/*
 * Input touch Device Driver Application
 *
 * (C) 2019.10.07 <buddy.zhang@aliyun.com>
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
#include <fcntl.h>
#include <linux/input.h>

/* Input sysfs direct */
#define INPUT_SYS_PATH	"/sys/devices/platform/soc/soc:input_touch/input"
/* Input direct on /dev/input/ */
static char input_dir[128] = "/dev/input";

int main()
{
	struct input_event ev;
	struct dirent *ptr;
	char events_dir[128];
	int find = 0;
	DIR *dir;
	int fd, rvl;

	/* 1st step
	 * --> Search input direct which contains events name on sysfs.
	 * --> Such path: /sys/devices/platform/Input_demo.1/input/inputx
	 */
	if ((dir = opendir(INPUT_SYS_PATH)) == NULL) {
		printf("Open %s dirent error.\n", INPUT_SYS_PATH);
		return -1;
	}

	/* Search INPUT_SYS_PATH/input direct */
	while ((ptr = readdir(dir)) != NULL) {
		if (ptr->d_type == DT_DIR) {
			if (!strncmp(ptr->d_name, "input", strlen("input"))) {
				/* Create full path name */
				strcpy(events_dir, INPUT_SYS_PATH);
				strcat(events_dir, "/");
				strcat(events_dir, ptr->d_name);
				find = 1;
				break;
			}
		}
	}

	/* close Current Dirent */
	closedir(dir);

	if (!find) {
		printf("Can't find %s/input\n", INPUT_SYS_PATH);
		return -1;
	}
	find = 0;

	/* 2nd step
	 * --> Search events name which points to /dev/input/eventx
	 */
	if ((dir = opendir(events_dir)) == NULL) {
		printf("Open %s dirent error.\n", events_dir);
		return -1;
	}

	/* Search INPUT_SYS_PATH/input direct */
	while ((ptr = readdir(dir)) != NULL) {
		if (ptr->d_type == DT_DIR) {
			if (!strncmp(ptr->d_name, "event", strlen("event"))) {
				strcpy(events_dir, ptr->d_name);
				find = 1;
				break;
			}
		}
	}

	/* close Current Dirent */
	closedir(dir);

	if (!find) {
		printf("Can't find events direct\n");
		return -1;
	}

	/* 3th step
	 * --> Open device node on /dev/input/eventx
	 */
	strcat(input_dir, "/");
	strcat(input_dir, events_dir);

	/* 4th step
	 * --> Read event from /dev/input/eventx
	 */
	fd = open(input_dir, O_RDWR);
	if (fd < 0) {
		printf("Unable to open %s\n", input_dir);
		return -1;
	}
	printf("Open Input Events: %s\n", input_dir);
	
	/* Read input events */
	while (1) {
		memset(&ev, 0, sizeof(struct input_event));
		/* read events */
		rvl = read(fd, &ev, sizeof(struct input_event));
		if (rvl != sizeof(struct input_event)) {
			printf("Can't read event frame\n");
			goto err_event;
		}
		/* output event information */
		printf("------------------------\n");
		printf("type:   %u\n", ev.type);
		printf("code:   %u\n", ev.code);
		printf("value:  %u\n", ev.value);
	}

	close(fd);
	return 0;

err_event:
	close(fd);
	return rvl;
}
