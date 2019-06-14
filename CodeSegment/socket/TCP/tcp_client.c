/*
 * Socket Client (TCP).
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
#define SOCKET_IP	"10.10.10.5"

/* Define Port */
#ifdef CONFIG_PORT
#define SOCKET_PORT	CONFIG_PORT
#else
#define SOCKET_PORT	8890
#endif

/* Buffer size */
#define MES_BUFSZ	10240

int main(void)
{
	struct sockaddr_in server_address;
	char sendbuf[MES_BUFSZ] = {0};
	char recvbuf[MES_BUFSZ] = {0};
	int server_len;
	int sock;	

	/* TCP: SOCK_STREAM */
	sock = socket(PF_INET, SOCK_STREAM, 0);
	server_address.sin_family = AF_INET;
	/* Configuration IP and Port */
	server_address.sin_addr.s_addr = inet_addr(SOCKET_IP);
	server_address.sin_port = htons(SOCKET_PORT);

	/* Bind */
	connect(sock, (struct sockaddr*)&server_address, 
						sizeof(server_address));

	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
		/* Send message */
		write(sock, sendbuf, sizeof(sendbuf));
		/* Write message */
		read(sock, recvbuf, sizeof(recvbuf));

		/* Prepare for next */
		memset(sendbuf, 0, sizeof(sendbuf));	
		memset(recvbuf, 0, sizeof(recvbuf));
	}

	close(sock);
	return 0;
}
