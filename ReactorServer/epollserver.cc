// epoll回声服务器
#include "EchoServer.h"
#include <sys/epoll.h>

int main(int argc, char *argv[])
{
    if(argc != 3) {
        printf("Usage: %s <ip> <port>.\n", argv[0]);
        printf("Example: ./server 192.168.150.128 5085\n\n");
        return -1;
    }

    EchoServer server(argv[1], atoi(argv[2]), 3, 0);
    server.Start();
    return 0;
}