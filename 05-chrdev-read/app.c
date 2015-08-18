#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    int fd, r_sz;
    char buf[10];

    fd = open("/dev/hello_driver", O_RDONLY);
    if(fd < 0){
        printf("open device node error\n");

        return -1;
    }

    memset(buf, 0, sizeof(buf));

    r_sz = read(fd, buf, sizeof(buf));
    if(r_sz > 0){
        printf("r_sz=%d content:%d\n", r_sz, buf[0]);
    }

    close(fd);

    return 0;

}
