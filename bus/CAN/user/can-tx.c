/*
 * CAN send message
 *
 * (C) 2018.12.18 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

int main(void)
{
    int s, nbytes;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame[2] = {{0}};
    
    /* Create can socket */
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr.ifr_name, "can0");
    /* set can device */
    ioctl(s, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    /* bind to can0 */
    bind(s, (struct sockaddr *)&addr, sizeof(addr));
    /* forbidden filter rule, only send message. */
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
    /* Create two message */
    frame[0].can_id  = 0x011;
    frame[0].can_dlc = 1;
    frame[0].data[0] = 0x23;
    frame[1].can_id  = 0x01;
    frame[1].can_dlc = 1;
    frame[1].data[0] = 'N';

    /* send meesage nop */
    while (1) {
        /* Send first message */
        nbytes = write(s, &frame[0], sizeof(frame[0]));
        if (nbytes != sizeof(frame[0])) {
            printf("Send Error frame[0] bytes %d\n", nbytes);
            break;
        } else
            printf("Write1 %d\n", nbytes);
        sleep(1);
        /* Send second message */
        nbytes = write(s, &frame[1], sizeof(frame[1]));
        if (nbytes != sizeof(frame[1])) {
            printf("Send Error frame[1] %d\n", nbytes);
            break;
        } else
            printf("Write2 %d\n", nbytes);
        sleep(1);
    }
    close(s);
    return 0;
}
