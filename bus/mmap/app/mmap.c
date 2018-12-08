#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <malloc.h>

#define PHYADDR               0x50000000

int main(void)
{
    unsigned int *map_base;
    FILE *f;
    int type, fd;

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("Fail to open /dev/mem.\n");
        return -1;
    }

    map_base = (unsigned int *)mmap(PHYADDR, 1024, PROT_READ | PROT_WRITE, 
                                                     MAP_SHARED, fd, 0);
    if (!map_base) {
        printf("Fail to mmap.\n");
        close(fd);
        return -1;
    }

    printf("map_base[10]: %#x\n", map_base[10]);
    map_base[10] = 0x20000;
    printf("map_base[10]: %#x\n", map_base[10]);

    close(fd);
    munmap(map_base, 1024);


    return 0;
}
