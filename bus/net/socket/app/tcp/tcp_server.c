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
    int server_sockfd, client_sockfd;	
    int server_len, client_len;
    struct sockaddr_in server_address, client_address;
    char recvbuf[1024];

    server_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("172.16.132.97");
    server_address.sin_port = htons(12345);
    server_len = sizeof(server_address);

    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, SOMAXCONN);	
    printf("SOMAXCONN:%d\n", SOMAXCONN);
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
