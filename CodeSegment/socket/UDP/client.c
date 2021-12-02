/*
 * Socket Client (UDP).
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
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>

/* Define IP */
#define SOCKET_IP	"127.0.0.1"

/* Define Port */
#define SOCKET_PORT	8810

int main(void)
{
	struct sockaddr_in server_address;
	char sendbuf[1024] = {0};
	int socket_fd;

	if ((socket_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror(" |-> Client CREATE SOCKET");
		exit(-1);
	}

	/* Configuration IP and Port on UDP */
	memset(&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(SOCKET_IP);
	server_address.sin_port = htons(SOCKET_PORT);

	/* Send Message */
	sprintf(sendbuf, "Hello %s", "BiscuitOS");
	printf(" |-> Client-Send: %s\n", sendbuf);
	sendto(socket_fd, sendbuf, strlen(sendbuf), 0,
			(struct sockaddr *)&server_address, sizeof(server_address));

	close(socket_fd);
	return 0;
}
