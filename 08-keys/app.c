#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main()
{
    int fd;
    unsigned long buf;

    fd = open("/dev/buttons", O_RDWR);

    if(fd < 0){
        perror("open failed");
        exit(-1);
    }

    while(1){
        sleep(1);
    memset(&buf, 0, sizeof(buf));
    read(fd, &buf, sizeof(buf));
    printf("%ld\n", buf);
    }


    close(fd);

    return 0;
}
