#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"
#include <cstdint>
#include <functional>
#include <mutex>
#include <sys/eventfd.h>

EventLoop::EventLoop():ep_(new Epoll), wakeupfd_(eventfd(0, EFD_NONBLOCK)), 
    wakeupChannel_(new Channel(this->epoll(), wakeupfd_)) {
    wakeupChannel_->enableRead();
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleWakeup, this));
}

void EventLoop::run()
{
    thread_id_ = syscall(SYS_gettid);
    while(true) {
        std::vector<Channel*> channels = ep_->loop();
        if(channels.size() == 0) {
            timeoutCallback_(this);
        }
        else {
            for(auto ch : channels) {
                ch->handleEvent();
            }
        }
    }
}

void EventLoop::addIOTask(std::function<void()> fun) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_tasks_.push(fun);
    }
    wakeup();
}

void EventLoop::wakeup() {
    uint64_t val=1;
    write(wakeupfd_,&val,sizeof(val));
}
void EventLoop::handleWakeup() {
    uint64_t val;
    read(wakeupfd_, &val, sizeof(val));

    std::function<void()> func;
    std::lock_guard<std::mutex> lock(mutex_);
    while(!queue_tasks_.empty()) {
        func = std::move(queue_tasks_.front());
        queue_tasks_.pop();
        func();
    }
}