// 客户端程序
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
int main(int argc, char* argv[])
{
    if(argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        printf("Example: ./client 192.168.150.128 5085\n\n");
        return -1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    char buf[1024];

    // 创建通信用的套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror("socket creation failed");
        return -1;
    }

    // 初始化服务端的地址，即接收数据的计算机的地址
    memset(&servaddr, 0, sizeof(sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    // TCP连接
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(sockaddr_in)) == -1) {
        printf("connect(%s : %s) failed\n", argv[0], argv[1]);
        close(sockfd);
        return -1;
    }

    printf("connect ok.\n");
    while(true) {
        // 将数据缓冲区清0
        memset(buf, 0, sizeof(buf));
        printf("input:");
        scanf("%s", buf);

        if(strcmp(buf, "exit") == 0) 
            break;

        // 发送数据
        if(send(sockfd, buf, sizeof(buf), 0) == -1) {
            perror("send failed.\n");
            close(sockfd);
            return -1;
        }
        // 接收数据
        memset(buf, 0, sizeof(buf));
        if(recv(sockfd, buf, sizeof(buf), 0) == -1) {
            perror("recv failed.\n");
            close(sockfd);
            return -1;
        }
        printf("recv: %s\n", buf);
    }

    return 0;
}