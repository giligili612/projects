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
#include "InetAddress.hpp"
#include "Socket.hpp"
#include "Epoll.hpp"
int main(int argc, char *argv[])
{
    if(argc != 3) {
        printf("Usage: %s <ip> <port>.\n", argv[0]);
        printf("Example: ./server 192.168.150.128 5085\n\n");
        return -1;
    }

    int listenfd = Socket::CreateNonBlock();
    Socket listen_sock = Socket(listenfd);
    listen_sock.setReuseAddr(true);
    listen_sock.setTcpNoDelay(true);
    listen_sock.setReusePort(true);
    listen_sock.setKeepAlive(true);

    InetAddress servaddr(argv[1], argv[2]);

    if(!listen_sock.bind(servaddr)) {
        puts("Socket bind error");
        exit(EXIT_FAILURE);
    }
    if(!listen_sock.listen(128)) {
        puts("Socket listen error");
        exit(EXIT_FAILURE);
    }

    // int epollfd = epoll_create(1);

    // epoll_event ev;
    // ev.data.fd = listenfd;
    // ev.events = EPOLLIN;

    // epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

    // epoll_event evs[10];
    Epoll epoll;
    epoll.addFd(listen_sock.fd(), EPOLLIN);
    std::vector<epoll_event> evs;
    while(true) {
        evs = epoll.loop();
        for(int i = 0; i < evs.size(); i++) {
            if(evs[i].events & EPOLLRDHUP) { // 对方已关闭写方向
                printf("client(eventfd=%d) disconnected.\n", evs[i].data.fd);
                close(evs[i].data.fd);
            } 
            else if(evs[i].events & (EPOLLIN | EPOLLPRI)){ // 接收缓冲区有数据可读
                if(evs[i].data.fd == listenfd) { // 有新客户端连上来
                    struct sockaddr_in addr;
                    socklen_t len = sizeof(addr);
                    InetAddress clientaddr(addr); // 存储客户端的地址
                    int clientfd = listen_sock.accept(clientaddr);
                    printf("accept client(fd=%d, ip=%s, port=%d) ok.\n", clientfd, clientaddr.ip(), clientaddr.port());
                    // 将套接字加入到epoll中监视
                    epoll.addFd(clientfd, EPOLLIN | EPOLLET);
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
                            epoll.delFd(evs[i].data.fd);           
                            break;
                        }
                    }
                }
            }
            else { // 其它事件，统一视为错误
                printf("client(eventfd=%d) error.\n", evs[i].data.fd);
                ::close(evs[i].data.fd);
            }
        }
    }
    return 0;
}