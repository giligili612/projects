#ifndef EPOLL_H
#define EPOLL_H
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstdio>
#include <vector>
#include <string.h>
#include <memory>

class Channel;
class Epoll
{
public:
    Epoll();
    ~Epoll();

    // 向Epoll里添加、更新监视的事件
    void updateChannel(Channel *ch);
    // 移除Channel
    void removeChannel(int fd);

    // epoll_wait的封装，返回已发生事件的集合
    std::vector<Channel*> loop(int timeout = -1);
    int fd() const { return epollfd_; }
private:
    static const int MaxEvents {100};    // 事件数组的大小
    int epollfd_ {-1};                   // 管理epoll的进程的FD
    epoll_event events_[MaxEvents];      // 事件数组
};
#endif