#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define FIFO_NAME      "/tmp/my_fifo"

int main()
{
    int fd;
    char buf[1];

    /* Open FIFO with read-only */
    fd = open(FIFO_NAME, O_RDONLY);
    if (fd < 0) {
        printf("Unable to open FIFO\n");
        return -1;
    }

    while (1) {
        memset(buf, 0, 1);
        read(fd, buf, 1);
        printf("%c", buf[0]);
    }
    /* Close FIFO */
    close(fd);

    return 0;
}
