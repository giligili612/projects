#ifndef CHANNEL_H
#define CHANNEL_H
#include <cstdint>
#include <sys/epoll.h>
#include <functional>
class Epoll;
class Socket;

class Channel
{
public:
    Channel(Epoll* ep, int fd): epoll_(ep), fd_(fd) {} // Channel指向Epoll,是否在Epoll中由Epoll自行决定
    ~Channel() {} // fd_和epoll_都不属于Channel，不要动这俩
    int fd() const { return fd_; }
    uint32_t events() const { return events_; }     // 正在监视的事件
    uint32_t rEvents() const { return revents_; }   // 已发生的事件
    bool isInEpoll() const { return isInEpoll_; }   // 是否被加入了epoll
    void setInEpoll(bool op) { isInEpoll_ = op; if(!op) epoll_ = nullptr; }
    void setET() { events_ = events_ | EPOLLET; }   // 边缘触发
    void setREvents(uint32_t ev) { revents_ = ev; } // 设置发生的事件
    void setEvents(uint32_t ev) { events_ = ev; }   // 设置要监视的事件
    void enableRead() { events_ |= EPOLLIN; }       // 允许epoll监视fd_的读事件
    void enableWrite() { events_ |= EPOLLOUT; }     // 允许epoll写事件
    void disableWrite() { events_ &= ~EPOLLOUT; }   // 不允许epoll写事件
    void handleEvent();
    
    // 设置回调函数的函数
    void setReadCallback(std::function<void()> func)  { readCallback_ = func; }
    void setWriteCallback(std::function<void()> func) { writeCallback_ = func; }
    void setCloseCallback(std::function<void()> func) { closeCallback_ = func; }
    void setErrorCallback(std::function<void()> func) { errorCallback_ = func; }
private:
    int fd_{-1};            // 标识一个socket或其他IO资源
    Epoll *epoll_{nullptr}; // fd_代表的socket通向事件循环的桥梁
    bool isInEpoll_{false};
    uint32_t events_{0};    // 要监视的事件
    uint32_t revents_{0};   // 已发生的事件
    std::function<void()> readCallback_;  // 读事件回调函数
    std::function<void()> writeCallback_; // 写事件回调函数
    std::function<void()> closeCallback_; // 关闭Channel回调函数
    std::function<void()> errorCallback_; // 关闭Channel回调函数
};
#endif