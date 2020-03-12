/*
 * Client in select
 *
 * (C) 2020.03.12 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>

int main(void)
{
	struct sockaddr_in client_addr;
	char buffer[BUFSIZ] = {0};
	int socket_fd = 0, max_fd = 0;
	int socket_len = 0, ret;
	struct timeval tv;
	fd_set read_fds;
	int retval;

	/* obtain socket descriptor */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		perror("create socket failed: ");
		return -1;
	}

	memset(&client_addr, 0, sizeof(struct sockaddr_in));

	/* init socket addr */
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(8080);
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	/* Connect service */
	ret = connect(socket_fd, (struct sockaddr *)&client_addr,
							sizeof(client_addr));
	if (ret < 0) {
		perror("connect: ");
		return -1;
	}

	if (socket_fd > max_fd)
		max_fd = socket_fd;

	while (1) {
		FD_ZERO(&read_fds);
		FD_SET(0, &read_fds);
		FD_SET(socket_fd, &read_fds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		retval = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
		if (retval < 0) {
			perror("select: ");
			return 0;
		}

		/* ACK? or response? */
		if (retval == 0)
			continue;

		memset(buffer, 0, BUFSIZ);
		/* Trigger response from keyboard */
		if (FD_ISSET(0, &read_fds)) {
			/* Read data from terminal */
			read(STDIN_FILENO, buffer, BUFSIZ);

			/* Send these data */
			socket_len = write(socket_fd, buffer, strlen(buffer));
			if (socket_len > 0)
				printf("Send message successful\n");
		}

		/* Trigger response from service */
		if (FD_ISSET(socket_fd, &read_fds)) {
			/* Send message from service */
			socket_len = read(socket_fd, buffer, BUFSIZ);
			if (socket_len > 0)
				printf("Read >> %s\n", buffer);
		}
	}
	return 0;
}
