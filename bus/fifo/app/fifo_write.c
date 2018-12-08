#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define FIFO_NAME      "/tmp/my_fifo"

int main()
{
    int fd;

    /* Verify wheterh FIFO exist? */
    if (access(FIFO_NAME, F_OK) == -1) {
        /* FIFO doesn't exist and establish a FIFO */
        if (mkfifo(FIFO_NAME, 0777) != 0) {
            printf("Could not create FIFO\n");
            return -1;
        }
    }

    /* Open FIFO with write-only */
    fd = open(FIFO_NAME, O_WRONLY);

    while (1) {
        write(fd, "A", 1);
    }
    /* Close FIFO */
    close(fd);

    return 0;
}
