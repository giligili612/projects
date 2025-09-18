#include "Channel.h"
#include "Epoll.h"
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <cerrno>


void Channel::handleEvent()
{
    // 对方已关闭写方向
    if(revents_ & EPOLLRDHUP) { 
        closeCallback_();
    } 
    // 接收缓冲区有数据可读
    else if(revents_ & (EPOLLIN | EPOLLPRI)){ 
        readCallback_();
    }
    // 写事件
    else if(revents_ & EPOLLOUT) {
        writeCallback_();
    }
    else { // 其它事件，统一视为错误
        printf("Other/error event detected for fd=%d, revents=%u\n", fd_, revents_);
        errorCallback_();
    }
}

