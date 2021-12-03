/*
 * Socket Server (UNIX).
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
#include <sys/stat.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

const char *filename = "BiscuitOS";

int main(void)
{
	struct sockaddr_un un;
	int sockfd;

	printf("** Unix Socket **\n");
	/* UNIX socket */
	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror(" |-> Server SOCKET ERROR.\n");
		exit(-1);
	}

	un.sun_family = AF_UNIX;
	unlink(filename);
	strcpy(un.sun_path, filename);

	/* Bind */
	if (bind(sockfd, (struct sockaddr *)&un, sizeof(un)) < 0) {
		perror(" |-> Server Bind ERROR.\n");
		goto err;
	}

	/* listen */
	if (listen(sockfd, 2) < 0) {
		perror(" |-> Server Listen ERROR.");
		goto err;
	}

	while (1) {
		struct sockaddr_un client_addr;
		char buffer[1024];
		int afd;

		memset(buffer, 0, 1024);
		/* accept */
		afd = accept(sockfd, NULL, NULL);
		if (afd < 0) {
			perror(" |-> Server Accept ERROR");
			goto err;
		}

		/* Recv */
		if (recv(afd, buffer, 1024, 0) < 0) {
			perror(" |-> Server Recv ERROR");
			goto err;
		}
		printf(" |-> Serv Recv: %s\n", buffer);
		close(afd);
	}

err:
	close(sockfd);
	return 0;
}
