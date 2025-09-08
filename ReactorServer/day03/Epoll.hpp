#ifndef EPOLL_H
#define EPOLL_H
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstdio>
#include <vector>
#include <string.h>
class Epoll
{
public:
    Epoll() {
        epollfd_ = epoll_create(1);
        if(epollfd_ == -1) {    
            puts("epoll_create failed.\n");
            exit(-1);   // epoll都能失败就别当服务器了
        }
    }

    ~Epoll() { if(epollfd_ > 0) ::close(epollfd_); }

    // 向Epoll里添加监视的事件
    void addFd(int fd, uint32_t op) {
        epoll_event ev;
        ev.data.fd = fd;
        ev.events = op;
        if(epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
            puts("Epoll::addFd epoll_ctl failed.\n");
        }
    }

    // 移除监视的事件
    void delFd(int fd) {
        printf("client(eventfd=%d) disconnected.\n", fd);
        epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, NULL);
        ::close(fd);
    }
    
    // epoll_wait的封装，返回已发生事件的集合
    std::vector<epoll_event> loop(int timeout = -1) {
        std::vector<epoll_event> evs;
        memset(events_, 0, sizeof(events_));
        int infds = epoll_wait(epollfd_, events_, MaxEvents, timeout);
        // 返回失败
        if(infds < 0) {
            perror("epoll_wait failed"); 
            exit(-1);
        }

        // 超时
        if(infds == 0) {
            perror("epoll_wait timeout");  
            exit(-1);
        }

        for(int i = 0; i < infds; i++) {
            evs.push_back(events_[i]);
        }
        return evs;
    }

    int fd() const { return epollfd_; }
private:
    static const int MaxEvents = 100;   // 事件数组的大小
    int epollfd_ {-1};                   // 管理epoll的进程的FD
    epoll_event events_[MaxEvents];      // 事件数组
};
#endif