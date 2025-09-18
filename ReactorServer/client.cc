// 客户端程序
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
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

    // printf("connect ok.\n");
    printf("connect ok at %ld\n", time(0));
    for (int ii=0;ii<100000;ii++)
    {
        memset(buf,0,sizeof(buf));
        sprintf(buf,"这是第%d个超级女生。",ii);

        char tmpbuf[1024];                 // 临时的buffer，报文头部+报文内容。
        memset(tmpbuf,0,sizeof(tmpbuf));
        int len=strlen(buf);                 // 计算报文的大小。
        memcpy(tmpbuf,&len,4);       // 拼接报文头部。
        memcpy(tmpbuf+4,buf,len);  // 拼接报文内容。

        int sent = send(sockfd,tmpbuf,len+4,0);  // 把请求报文发送给服务端。
        // printf("DEBUG: sent %d bytes, message length=%d, content='%s'\n", sent, len, buf);

        if(sent < 0) {
            perror("send failed");
            break;
        }
    
        // 添加短暂延迟，避免发送过快
        usleep(10); // 10ms delay
    }
    // for (int ii=0;ii<100;ii++)
    // {
    //     int len;
    //     recv(sockfd,&len,4,0);            // 先读取4字节的报文头部。

    //     memset(buf,0,sizeof(buf));
    //     recv(sockfd,buf,len,0);           // 读取报文内容。

    //     printf("recv:%s\n",buf);
    // }
    printf("connect closed at %ld\n", time(0));
    return 0;
}