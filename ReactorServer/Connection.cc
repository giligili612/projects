#include "Connection.h"
#include "Channel.h"
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>


Connection::Connection(EventLoop *evLoop, std::unique_ptr<Socket> socket)
        :evLoop_(evLoop), clientSocket_(std::move(socket))
{
    clientChannel_ = std::make_unique<Channel>(evLoop_->epoll(), clientSocket_->fd());
    clientChannel_->setET();
    clientChannel_->enableRead();
    clientChannel_->setReadCallback(std::bind(&Connection::recvMessage, this));
    clientChannel_->setWriteCallback(std::bind(&Connection::send, this));
    clientChannel_->setCloseCallback(std::bind(&Connection::closeChannel, this));
    clientChannel_->setErrorCallback(std::bind(&Connection::errorChannel, this));
    evLoop_->epoll()->updateChannel(clientChannel_.get());
}

Connection::~Connection()
{}

void Connection::recvMessage()
{
    char buf[1024];
    while(true) {
        memset(buf, 0, sizeof(buf));
        ssize_t nread = read(clientChannel_->fd(), buf, sizeof(buf));
        if(nread > 0) { // 读取到数据
            recvBuf_.append(buf, nread);
            while(recvBuf_.size() >= 4) {   // 可能有一条完整消息
                int len;
                memcpy(&len, recvBuf_.data(), 4);
                if(recvBuf_.size() - 4 < len) { // 还不构成一条完整的消息，继续接收
                    break;
                }
                // 构成了一条完整消息，接收发给TcpServer处理，并清理接收缓冲区的这条消息
                std::string message(recvBuf_.data() + 4, len);
                recvBuf_.erase(0, len + 4);
                recvMessageCallback_(shared_from_this(), message);
            }
            
        }
        else if(0 == nread) { // 连接已断开
            closeChannel();       
            break;
        }
        else if(-1 == nread) {
            if(errno == EINTR) // 读数据时被信号中断，继续读取
                continue;
            else if(errno == EAGAIN || errno == EWOULDBLOCK) {  // 全部数据读取完毕
                break;
            }
        }
        else {      // 其他事件均视为错误
            perror("read error");
            closeChannel();
            break;
        }   
    }
}

void Connection::closeChannel()
{
    disconnect_ = true;
    evLoop_->epoll()->removeChannel(clientChannel_->fd());
    clientChannel_->setInEpoll(false);
    closeConnectionCallback_(clientSocket_->fd());
}

void Connection::errorChannel()
{
    disconnect_ = true;
    closeConnectionCallback_(clientSocket_->fd());
}

void Connection::write(const std::string &message)
{
    if(disconnect_) return;
    // 解决的问题：如果IO线程正在发送数据时，工作线程修改了Buffer
    // 
    if(evLoop_->isEventLoop()) { // 如果当前线程是IO线程
        sendInEventLoop(message);
    }else { // 如果当前线程不是IO线程
        evLoop_->addIOTask(std::bind(&Connection::sendInEventLoop, this, message));
    }
}

void Connection::send()
{
    if(disconnect_) return;
    int sendedLen = ::send(clientSocket_->fd(), sendBuf_.data(), sendBuf_.size(), 0);
    if(sendedLen > 0) sendBuf_.erase(0, sendedLen);
    if(0 == sendBuf_.size()) clientChannel_->disableWrite();
    sentCompleteCallback_(shared_from_this());
}

void Connection::sendInEventLoop(const std::string &message) {
    // 写入本条消息的长度
    int len = message.size();
    sendBuf_.append((const char*)&len, 4);
    // 写入消息
    sendBuf_.append(message.data(), len);
    
    
    // 允许输出消息
    clientChannel_->enableWrite();
    evLoop_->epoll()->updateChannel(clientChannel_.get());
}
