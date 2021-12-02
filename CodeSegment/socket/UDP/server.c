/*
 * Socket Server (UDP).
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

/* Define PORT */
#define SOCKET_PORT	 8810

int main(void)
{
	struct sockaddr_in server_address, client_address;
	int server_sockfd, client_sockfd;	
	int server_len, client_len;
	char recvbuf[1024];
	int ret;

	printf("** UDP Socket **\n");
	/* UDP socket */
	if ((server_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror(" |-> Server SOCKET ERROR.\n");
		exit(-1);
	}
	
	/* Configureation IP and PORT on UDP */
	server_len = sizeof(struct sockaddr_in);
	memset(&server_address, 0, server_len);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(SOCKET_PORT);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_len = sizeof(struct sockaddr_in);

	/* Bind */
	if ((ret = bind(server_sockfd, 
			(struct sockaddr *)&server_address, server_len)) < 0) {
		printf(" |-> Server BIND ERROR. %d ret\n", ret);
		close(server_sockfd);
		exit(-1);
	}
	printf(" |-> Server Bind Succeed.\n");

	/* listen */
	while (1) {
		memset(recvbuf, 0, sizeof(recvbuf));
		client_len = recvfrom(server_sockfd, recvbuf, sizeof(recvbuf),
				0, (struct sockaddr *)&client_address, &server_len);
		if (client_len) {
			printf(" |-> Server-Resv: %s\n", recvbuf);
			sendto(server_sockfd, recvbuf, server_len, 0,
				 (struct sockaddr *)&client_address, server_len);
		}
	}

	close(server_sockfd);
	close(client_sockfd);
	return 0;
}
