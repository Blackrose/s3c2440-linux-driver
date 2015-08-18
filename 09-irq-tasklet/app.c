#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main()
{
    int fd, r_sz, i;
    int buf[6];

    fd = open("/dev/buttons", O_RDWR);

    if(fd < 0){
        perror("open failed");
        exit(-1);
    }

    while(1){
        sleep(1);
        memset(buf, 0, sizeof(buf));
        r_sz = read(fd, buf, sizeof(buf));
        if(r_sz < 0)
            break;
        printf("*********\n");
        for(i = 0; i < 6; i++)
            printf("%d\n", buf[i]);
    }


    close(fd);

    return 0;
}
