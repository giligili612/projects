#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <asm-generic/socket.h>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <unistd.h>
#include "InetAddress.hpp"

class Socket
{
public:
    // 创建一个用于监听的非阻塞套接字，返回该套接字的文件描述符
    static const int CreateNonBlock() {
        int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
        if(listenfd == -1) { // 连创建套接字都做不到的服务器直接杀掉，no mercy
            perror("socket creation failed");
            exit(-1);
        }
        return listenfd;
    }
    Socket(int fd):fd_(fd) {}
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    // ! 还不知道是否要用到移动语义

    ~Socket() { if(fd_ > 0) ::close(fd_); }

    void setReuseAddr(bool on) const {
        int opt = on ? 1 : 0;
        ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));
    }
    void setTcpNoDelay(bool on) const {
        int opt = on ? 1 : 0;
        ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof opt));
    }
    void setReusePort(bool on) const {
        int opt = on ? 1 : 0;
        ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));
    }
    void setKeepAlive(bool on) const {
        int opt = on ? 1 : 0;
        ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt));
    }

    bool bind(InetAddress &servaddr) const {
        if(::bind(fd_, servaddr.addr(), sizeof(sockaddr)) == -1) {
            perror("bind failed");
            ::close(fd_);
            return false;
        }
        return true;
    }

    bool listen(int backlog) const {
        if(::listen(fd_, backlog) == -1) {
            perror("listen failed");
            ::close(fd_);
            return false;
        }
        return true;
    }

    int accept(InetAddress &clientaddr) const {
        sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int clientfd = accept4(fd_, (struct sockaddr*)&addr, &len, SOCK_NONBLOCK);
        clientaddr.setAddr(addr);
        return clientfd;
    }

    int fd() const { return fd_; }
private:
    int fd_{-1};
};

#endif