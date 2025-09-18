#include "Acceptor.h"
#include "Channel.h"
#include "EventLoop.h"
#include <cstdio>
#include <functional>
#include <memory>
Acceptor::Acceptor(EventLoop *evLoop, const std::string& ip, uint16_t port)
    :evLoop_(evLoop)
{
    int sockfd = Socket::CreateNonBlock();
    socket_ = std::make_unique<Socket>(sockfd);
    socket_->setReuseAddr(true);
    socket_->setTcpNoDelay(true);
    socket_->setReusePort(true);
    socket_->setKeepAlive(true);

    InetAddress servaddr(ip, port);

    if(!socket_->bind(servaddr)) {
        puts("Socket bind error");
        exit(EXIT_FAILURE);
    }
    if(!socket_->listen(128)) {
        puts("Socket listen error");
        exit(EXIT_FAILURE);
    }

    // 专门用于接收连接的Channel
    listenChannel_ = std::make_unique<Channel>(evLoop_->epoll(), sockfd);
    listenChannel_->setET();
    listenChannel_->setReadCallback(std::bind(&Acceptor::acceptConnection, this));
    listenChannel_->enableRead();
    evLoop_->epoll()->updateChannel(listenChannel_.get());
}

Acceptor::~Acceptor()
{

}

void Acceptor::acceptConnection()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int clientfd = accept4(socket_->fd(), (struct sockaddr*)&addr, &len, SOCK_NONBLOCK);
    InetAddress clientaddr(addr); // 存储客户端的地址
    // 给相应客户端新建一个Socket
    std::unique_ptr<Socket>clientSock = std::make_unique<Socket>(clientfd, clientaddr.ip(), clientaddr.port());
    // printf("create new socket for new connection fd=%d\n", clientfd);
    // 调用回调函数
    cnnCb(std::move(clientSock));
}