// epoll回声服务器
#include <asm-generic/socket.h>
#include <cerrno>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>

int main(int argc, char *argv[])
{
    if(argc != 3) {
        printf("Usage: %s <ip> <port>.\n", argv[0]);
        printf("Example: ./server 192.168.150.128 5085\n\n");
        return -1;
    }

    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if(listenfd == -1) {
        perror("socket creation failed");
        return -1;
    }

    int opt = 1;
    // SO_REUSEADDR：允许重用本地地址，在连接进入TIME_WAIT阶段时允许快速重启以接收新的连接
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));
    // TCP_NODELAY: 仅用Nagle算法，对降低延迟至关重要
    setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof opt));
    // SO_REUSEPORT: 允许多个套接字绑定到相同的地址和端口
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));
    // SO_KEEPALIVE: 启用TCP保活机制，如果一段时间内连接无数据流动，会发送保活探测包检测对端是否还在，不在就断开连接
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt));

    // 初始化服务器地址
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    // 绑定socket的地址
    if(bind(listenfd, (sockaddr*)&servaddr, sizeof(sockaddr_in)) == -1) {
        perror("bind failed");
        close(listenfd);
        return -1;
    }

    // 开始监听
    if(listen(listenfd, 128) == -1) {
        perror("listen failed");
        close(listenfd);
        return -1;
    }

    int epollfd = epoll_create(1);

    struct epoll_event ev;
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;

    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

    struct epoll_event evs[10];

    while(true) {
        int infds = epoll_wait(epollfd, evs, 10, -1);
        // 返回失败
        if(infds < 0) {
            perror("epoll_wait failed");  break;
        }

        // 超时
        if(infds == 0) {
            perror("epoll_wait timeout");  break;
        }

        for(int i = 0; i < infds; i++) {
            if(evs[i].events & EPOLLRDHUP) { // 对方已关闭写方向
                printf("client(eventfd=%d) disconnected.\n", evs[i].data.fd);
                close(evs[i].data.fd);
            } 
            else if(evs[i].events & (EPOLLIN | EPOLLPRI)){ // 接收缓冲区有数据可读
                if(evs[i].data.fd == listenfd) { // 有新客户端连上来
                    struct sockaddr_in clientaddr;
                    socklen_t len = sizeof(clientaddr);
                    int clientfd = accept4(listenfd, (struct sockaddr*)&clientaddr, &len, SOCK_NONBLOCK);

                    printf("accept client(fd=%d, ip=%s, port=%d) ok.\n", clientfd, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                    // 为新客户端添加读事件，并添加到epoll中
                    ev.data.fd = clientfd;
                    ev.events = EPOLLIN | EPOLLET;  // 边缘触发
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
                }
                else { // 连接发来了新的数据
                    char buf[1024];
                    while(true) {
                        memset(buf, 0, sizeof(buf));
                        ssize_t nread = read(evs[i].data.fd, buf, sizeof(buf));
                        if(nread > 0) {
                            printf("recv from %d: %s\n", evs[i].data.fd, buf);
                            send(evs[i].data.fd, buf, nread, 0);
                        }
                        else if(nread == -1 && errno == EINTR) { // 读数据时被信号中断，继续读取
                            continue;
                        }
                        else if(nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) { // 全部数据已读取完毕
                            break;
                        }
                        else if(nread == 0) { // 连接已断开
                            printf("2client(eventfd=%d) disconnected.\n",evs[i].data.fd);
                            epoll_ctl(epollfd, EPOLL_CTL_DEL, evs[i].data.fd, NULL);
                            close(evs[i].data.fd);            
                            break;
                        }
                    }
                }
            }
            else { // 其它事件，统一视为错误
                printf("client(eventfd=%d) error.\n", evs[i].data.fd);
                close(evs[i].data.fd);
            }
        }
    }
    return 0;
}