#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_LENGTH 1024
static char msgBuffer[BUFFER_LENGTH];

int main()
{
    int ret, fd;
    char toSend[BUFFER_LENGTH] = {0};
    printf("\n");

    // 1. 打开设备 0
    fd = open("/dev/chardev0", O_RDWR);
    if (fd < 0)
    {
        perror("failed to open /dev/chardev0 device.\n");
        return -1;
    }

    // 吸入除了换行符以外的字符
    printf("type something to the kernel module:\n");
    scanf("%[^\n]%*c", toSend);
    printf("writing message to the kernel.\n");

    // 2. 向设备中写入数据
    ret = write(fd, toSend, strlen(toSend));
    if (ret < 0)
    {
        perror("failed to write the message\n");
        return -1;
    }

    // 3. 回车从设备中读数据
    printf("Press Enter to read back from the device.\n");
    getchar();

    printf("read from the device.\n");
    ret = read(fd, msgBuffer, strlen(toSend));
    if (ret < 0)
    {
        perror("failed to read message from the device.\n");
        return -1;
    }

    printf("the msg from device is:[%s]\n", msgBuffer);

    return 0;
}