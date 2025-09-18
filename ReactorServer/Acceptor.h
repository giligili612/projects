#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "Socket.h"
#include "EventLoop.h"
#include "Channel.h"
#include <functional>
#include <memory>

class Acceptor
{
public:
    Acceptor(EventLoop *evLoop, const std::string& ip, uint16_t port);
    ~Acceptor();
    void acceptConnection();
    void setNewConnectionSlot(std::function<void(std::unique_ptr<Socket>)> fn) { cnnCb = fn; }
private:
    EventLoop *evLoop_;
    std::unique_ptr<Socket> socket_;
    std::function<void(std::unique_ptr<Socket>)> cnnCb;
    std::unique_ptr<Channel> listenChannel_;
};

#endif