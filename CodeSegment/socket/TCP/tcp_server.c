/*
 * Socket Server (TCP).
 *
 * (C) 2019.06.10 <buddy.zhang@aliyun.com>
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

/* Define PORT */
#ifdef CONFIG_PORT
#define SOCKET_PORT	CONFIG_PORT
#else
#define SOCKET_PORT	8890
#endif

int main(void)
{
	struct sockaddr_in server_address, client_address;
	int server_sockfd, client_sockfd;	
	int server_len, client_len;
	char recvbuf[1024];

	/* TCP socket */
	server_sockfd = socket(PF_INET, SOCK_STREAM, 0);
	server_address.sin_family = AF_INET;
	/* Configureation IP and PORT */
	server_address.sin_addr.s_addr = inet_addr(SOCKET_IP);
	server_address.sin_port = htons(SOCKET_PORT);
	server_len = sizeof(server_address);

	/* Bind */
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	/* listen */
	listen(server_sockfd, SOMAXCONN);	
	client_len=sizeof(client_address);
	client_sockfd=accept(server_sockfd, 
		(struct sockaddr *)&client_address, (socklen_t *)&client_len);

	while(1) {
		memset(recvbuf,0,sizeof(recvbuf));
		read(client_sockfd, recvbuf, sizeof(recvbuf));
		fputs(recvbuf,stdout);
		write(client_sockfd, recvbuf, sizeof(recvbuf));
	}

	close(server_sockfd);
	close(client_sockfd);
	return 0;
}
