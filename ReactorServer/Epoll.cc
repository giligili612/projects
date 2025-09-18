#include "Epoll.h"
#include "Channel.h"
#include <cstdio>
#include <sys/epoll.h>

Epoll::Epoll() 
{
    epollfd_ = epoll_create(1);
    if(-1 == epollfd_) {    
        puts("epoll_create failed.\n");
        exit(-1);   // epoll都能失败就别当服务器了
    }
}

Epoll::~Epoll() 
{ 
    if(epollfd_ > 0) 
    ::close(epollfd_); 
}

void Epoll::updateChannel(Channel *ch)
{
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->events();
    if(ch->isInEpoll()) {   // Channel已经在树上了
        if(-1 == epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev)) {
            perror("Epoll::updateChannel epoll_ctl EPOLL_CTL_MOD failed.");
            exit(-1);
        }
    }
    else {  // Channel不在树上了
        if(-1 == epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev)) {
            perror("Epoll::updateChannel epoll_ctl EPOLL_CTL_ADD failed.");
            exit(-1);
        }
        ch->setInEpoll(true);
    }
    
}

void Epoll::removeChannel(int fd) 
{
    epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, NULL);
}

// 返回在epoll树上的发生事件的channel
std::vector<Channel*> Epoll::loop(int timeout)
{
    std::vector<Channel*> activeChannels;  // 存放发生的事件

    memset(events_, 0, sizeof(events_));
    int infds = epoll_wait(epollfd_, events_, MaxEvents, timeout);
    // 返回失败
    if(infds < 0) {
        perror("epoll_wait failed"); 
        exit(-1);
    }

    // 超时
    if(0 == infds) {
        return activeChannels;  
    }

    for(int i = 0; i < infds; i++) {
        Channel* ch = (Channel*)events_[i].data.ptr;
        ch->setREvents(events_[i].events);
        activeChannels.push_back(ch);
    }
    return activeChannels;
}