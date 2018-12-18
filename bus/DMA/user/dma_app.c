/*
 * DMA application
 *
 * (C) 2018.11.29 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>

#define DRIVER_NAME 		"/dev/axidma"

#define AXIDMA_IOC_MAGIC        'A'
#define AXIDMA_IOCGETCHN        _IO(AXIDMA_IOC_MAGIC, 0)
#define AXIDMA_IOCCFGANDSTART   _IO(AXIDMA_IOC_MAGIC, 1)
#define AXIDMA_IOCGETSTATUS     _IO(AXIDMA_IOC_MAGIC, 2)
#define AXIDMA_IOCRELEASECHN    _IO(AXIDMA_IOC_MAGIC, 3)

#define DMA_STATUS_UNFINISHED   0
#define DMA_STATUS_FINISHED     1

struct axidma_chncfg {
    unsigned int src_addr;
    unsigned int dst_addr;
    unsigned int len;
    unsigned char chn_num;
    unsigned char status;
    unsigned char reserve[2];
    unsigned int reserve2;
};

#define SRC_ADDR         0x60000000
#define DST_ADDR         0x70000000
#define DMA_MEMCPY_LEN     0x300000

int main(void)
{
    struct axidma_chncfg chncfg;
    int fd = -1;
    int ret;
    
    printf("AXI dma test, only support mem to mem: copy %#lx to %#lx,"
                      " size:3M\n", SRC_ADDR, DST_ADDR);
    /* open dev */
    fd = open(DRIVER_NAME, O_RDWR);
    if(fd < 0) {
        printf("open %s failed\n", DRIVER_NAME);
        return -1;
    }
    
    /* get channel */
    ret = ioctl(fd, AXIDMA_IOCGETCHN, &chncfg);
    if(ret){
        printf("ioctl: get channel failed\n");
        goto error;
    }
    printf("channel: %d\n", chncfg.chn_num);
 
    /* config addr */
    chncfg.src_addr = SRC_ADDR;
    chncfg.dst_addr = DST_ADDR;
    chncfg.len = DMA_MEMCPY_LEN;
    ret = ioctl(fd, AXIDMA_IOCCFGANDSTART, &chncfg);
    if(ret) {
        printf("ioctl: config and start dma failed\n");
        goto error;
    }
 
    /* wait finish */
    while(1) {
        ret = ioctl(fd, AXIDMA_IOCGETSTATUS, &chncfg);
        if(ret) {
            printf("ioctl: get status failed\n");
            goto error;
        }
        if (DMA_STATUS_FINISHED == chncfg.status) {
            break;
        }
        printf("status:%d\n", chncfg.status);
        sleep(1);
    }
 
    /* release channel */
    ret = ioctl(fd, AXIDMA_IOCRELEASECHN, &chncfg);
    if(ret) {
        printf("ioctl: release channel failed\n");
        goto error;
    }
 
    close(fd);
 
    return 0;
error:
    close(fd);
    return -1;
}
