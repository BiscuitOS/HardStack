/*
 * CAN receive message
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
    struct can_frame frame;
    struct can_filter rfilter[1];

    /* Create socket */
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr.ifr_name, "can0");
    ioctl(s, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    /* bind socket to can0 */
    bind(s, (struct sockaddr *)&addr, sizeof(addr));
    /* define rule to filter can frame */
    rfilter[0].can_id = 0x11;
    rfilter[0].can_mask = CAN_SFF_MASK;
    /* setup filter rule */
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
    /* monitoring can0 */
    printf("|    ID    |     DLC     |  DATA[0]\n");
    while (1) {
        /* Receive message */
        nbytes = read(s, &frame, sizeof(frame));
        /* dump message */
        if (nbytes > 0)
            printf("%#x %#x %#x\n", frame.can_id,
                   frame.can_dlc, frame.data[0]);
    }
    close(s);
    return 0;
}
