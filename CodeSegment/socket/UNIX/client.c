/*
 * Socket Client (UNIX).
 *
 * (C) 2021.12.03 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

const char *filename = "BiscuitOS";

int main(void)
{
	struct sockaddr_un un;
	char buffer[1024];
	int sockfd;

	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, filename);

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror(" |-> Client Sock ERROR");
		exit(-1);
	}

	if (connect(sockfd, (struct sockaddr *)&un, sizeof(un)) < 0) {
		perror(" |-> Client connect ERROR");
		goto err;
	}

	/* Send Buffer */
	sprintf(buffer, "Hello %s", "BiscuitOS");
	send(sockfd, buffer, 1024, 0);

err:
	close(sockfd);
	return 0;
}
