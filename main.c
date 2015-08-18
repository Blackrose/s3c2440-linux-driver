#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mem.h"

int main()
{
    int fd;
    char buf[100];

    fd = open("/dev/test", O_RDWR);

    if(fd < 0){
        perror("open failed");
        exit(-1);
    }

    memset(buf, 0, sizeof(buf));
    read(fd, buf, 100);

    printf("%s\n", buf);

    ioctl(fd, HELLO_CMD_WEL, NULL);
    
    memset(buf, 0, sizeof(buf));
    read(fd, buf, 100);

    printf("%s\n", buf);


    close(fd);

    return 0;
}
