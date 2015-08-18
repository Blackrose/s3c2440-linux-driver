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
    int fd;
    int buf;
        
    fd = open("/dev/hello_dev_node", O_RDWR);
    if (fd < 0)
    {
        printf("open /dev/hello_dev_node failed!\n");
        printf("%s\n", strerror(errno));
        return -1;
    }
    printf("open /dev/hello_dev_node ok!\n");
   
    memset(&buf, 0, sizeof(buf));
    read(fd, &buf, sizeof(buf));
    printf("before read:%d\n", buf);
   
    memset(&buf, 0, sizeof(buf));
    buf = 10;
    write(fd, &buf, sizeof(buf));

    memset(&buf, 0, sizeof(buf));
    read(fd, &buf, sizeof(buf));
    printf("after read:%d\n", buf);

    close(fd);
                                                                                                    
    return 0;
                                                                                                    
}

