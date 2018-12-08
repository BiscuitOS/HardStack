#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>

int main(void)
{
    int sock;	
    int server_len;
    char sendbuf[1024]={0};
    char recvbuf[1024]={0};
    struct sockaddr_in server_address;
	
    sock = socket(PF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("172.16.132.97");
    server_address.sin_port = htons(12345);
    connect(sock, (struct sockaddr*)&server_address, sizeof(server_address));

    while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
        write(sock, sendbuf, sizeof(sendbuf));
        read(sock, recvbuf, sizeof(recvbuf));
        fputs(recvbuf, stdout);
        memset(sendbuf, 0, sizeof(sendbuf));	
        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(sock);
    return 0;
}
