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
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

int main()
{
	struct sockaddr_in serv_addr, client_addr;
	int socket_fd = 0, client_fd = 0;
	char buffer[BUFSIZ] = {0};
	int read_len, select_count = 0;
	int reuse = 1, max_fd = 0;
	int select_fd[100] = {0};
	char IP[100][20] = {0};
	int ret = 0, index, retval;
	socklen_t socket_len;
	fd_set read_fds;
	struct timeval tv;

	/* Create socket descriptor */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(8080);

	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, 
			(const char *)&reuse, sizeof(reuse));

	/* Bind socket descriptor with host address */
	ret = bind(socket_fd, (struct sockaddr *)&serv_addr,
						sizeof(serv_addr));
	if (ret < 0) {
		perror("Bind failed!");
		close(socket_fd);
		return -1;
	}
	
	/* setup listen state */
	listen(socket_fd, 1024);

	FD_ZERO(&read_fds);
	FD_SET(socket_fd, &read_fds);
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	max_fd = socket_fd;

	while (1) {
		FD_ZERO(&read_fds);
		FD_SET(socket_fd, &read_fds);
		tv.tv_sec = 10;
		tv.tv_usec = 0;
		max_fd = socket_fd;

		/* collection into set */
		for (index = 0; index < select_count; index++) {
			FD_SET(select_fd[index], &read_fds);
			if (select_fd[index] > max_fd)
				max_fd = select_fd[index];
		}

		retval = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
		if (retval < 0)
			perror("select");

		/* No response */
		if (!retval)
			continue;

		memset(buffer, 0, BUFSIZ);

		/* Determine which client response */
		for (index = 0; index < select_count; index++) {
			if (FD_ISSET(select_fd[index], &read_fds)) {
				read_len = 
					read(select_fd[index], buffer, BUFSIZ);

				/* Re-response to send */
				write(select_fd[index], buffer, strlen(buffer));
				if (read_len > 0)
					printf("Read data from: %s: %s\n",
						IP[index], buffer);
			}
		}

		/* New client connect service */
		if (FD_ISSET(socket_fd, &read_fds)) {
			/* listen connect and create socket when new connect */
			socket_len = sizeof(struct sockaddr_in);
			client_fd = accept(socket_fd, 
					  (struct sockaddr *)&client_addr,
					  &socket_len);
			if (client_fd < 0) {
				perror("Accept error: ");
				return -1;
			} else {
				select_fd[select_count] = client_fd;

				/* Bind with IP */
				strncpy(IP[select_count], 
					inet_ntoa(client_addr.sin_addr), 
					20);
				select_count++;

				printf("Connect %s %d successful\n",
					inet_ntoa(client_addr.sin_addr),
					ntohs(client_addr.sin_port));
			}
		}
	}
	close(socket_fd);

	return 0;
}
