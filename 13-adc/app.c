#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
int main(int argc, char **argv)
{
    int fd, i, pid;
    int buf = 0, w_sz, cnt = 0;
       
    pid = getpid();

    fd = open("/dev/leds", O_RDWR);
    if (fd < 0)
    {
        printf("open /dev/hello_dev_node failed!\n");
        printf("%s\n", strerror(errno));
        return -1;
    }
    printf("open /dev/hello_dev_node ok!\n");

    while(1){
        sleep(1);
        if(cnt++ %2)
            buf = 0;
        else
            buf = 1;

    w_sz = write(fd, &buf, sizeof(buf));
    if(w_sz <= 0){
        printf("write error, %d\n", w_sz);
    }

    if(cnt > 100)
        cnt = 0;

    }

    close(fd);
    return 0;
}

