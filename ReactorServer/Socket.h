#ifndef SOCKET_H
#define SOCKET_H
#include <asm-generic/socket.h>
#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include "InetAddress.h"

class Socket
{
public:
    // 创建一个用于监听的非阻塞套接字，返回该套接字的文件描述符
    static const int CreateNonBlock() {
        int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
        if(-1 == listenfd) { // 连创建套接字都做不到的服务器直接杀掉，no mercy
            perror("socket creation failed");
            exit(-1);
        }
        return listenfd;
    }
    Socket(int fd):fd_(fd) {}
    Socket(int fd, std::string_view ip, uint16_t port):fd_(fd), ip_(ip), port_(port) {}
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
    // 将Socket绑定到 ip:port
    bool bind(InetAddress &addr) {
        if(-1 == ::bind(fd_, addr.addr(), sizeof(sockaddr))) {
            perror("bind failed");
            ::close(fd_);
            return false;
        }
        ip_ = addr.ip();
        port_ = addr.port();
        return true;
    }

    bool listen(int backlog) const {
        if(-1 == ::listen(fd_, backlog)) {
            perror("listen failed");
            ::close(fd_);
            return false;
        }
        return true;
    }

    int fd() const { return fd_; }
    std::string ip() const { return ip_; }
    uint16_t port() const { return port_; }
private:
    int fd_{-1};
    std::string ip_{};
    uint16_t port_;
};

#endif