#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include "Channel.h"
#include "Epoll.h"
#include "Socket.h"
#include <functional>
#include <queue>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <mutex>
#include <memory>

class EventLoop
{
public:
    EventLoop();
    ~EventLoop() { delete ep_; }
    void run();
    Epoll* epoll() { return ep_; }
    void setTimeOutCallback(std::function<void(EventLoop*)> func) { timeoutCallback_ = func; }
    bool isEventLoop() { return thread_id_ == syscall(SYS_gettid); }   // 判断当前线程是否为事件循环线程
    void addIOTask(std::function<void()> fun);
    void wakeup();
    void handleWakeup();
private:
    Epoll *ep_;              // 一个EventLoop只有一个Epoll
    std::function<void(EventLoop*)> timeoutCallback_;
    pid_t thread_id_;       // 线程id
    std::queue<std::function<void()>> queue_tasks_;     // IO任务队列
    std::mutex mutex_;      // IO任务队列的锁
    int wakeupfd_;
    std::unique_ptr<Channel> wakeupChannel_;
};

#endif