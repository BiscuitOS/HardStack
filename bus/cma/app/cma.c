/*
 * CMA application
 *
 * (C) 2018.11.29 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define CMA_MEM_VERSION         3
#define CMEM_IOCTL_MAGIC        'm'
#define CMEM_ALLOCATE           _IOW(CMEM_IOCTL_MAGIC, 1, unsigned int)
#define CMEM_RELEASE            _IOW(CMEM_IOCTL_MAGIC, 2, unsigned int)
 
struct cmamem_info {
    unsigned int version;
    unsigned int len;
    unsigned int offset;
    unsigned int mem_base;
    unsigned int phy_base;
};
 
int main()
{
    int cmem_fd;
    void *cmem_base;
    unsigned int size;
    struct cmamem_info region;

    cmem_fd = open("/dev/cma_mem", O_RDWR, 0);
    if (cmem_fd < 0) {
        perror("Can't open /dev/CMA\n");
        return -1;
    }       
    memset(&region, 0x00, sizeof(struct cmamem_info));
    region.version = CMA_MEM_VERSION;
    region.len = 1 << 20;

    if (ioctl(cmem_fd, CMEM_ALLOCATE, &region) < 0) {	
        perror("PMEM_GET_TOTAL_SIZE failed\n");
        return -1;
    }
    size = region.len;
    cmem_base = mmap(0, size, PROT_READ | PROT_WRITE, 
                                  MAP_SHARED, cmem_fd, region.phy_base);
    printf("CMA[%d] Phyaddress: %#08x Virtual base: %#08x "
           "Region.len: %#08x Offset: %#08x\n", i,
               (unsigned int)region.phy_base,
               (unsigned int)cmem_base, 
               (unsigned int)region.len, 
               (unsigned int)region.offset);
    if (cmem_base == MAP_FAILED) {
        cmem_base = 0;
        ioctl(cmem_fd, CMEM_RELEASE, &region)
        close(cmem_fd);
        cmem_fd = -1;
        perror("mmap pmem error!\n");
    }

    close(cmem_fd);
    return 0;
}
