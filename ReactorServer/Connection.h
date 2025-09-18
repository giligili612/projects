#ifndef CONNECTION_H
#define CONNECTION_H
#include "Buffer.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

class Connection;
using shared_ptr_conn = std::shared_ptr<Connection>;

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    Connection(EventLoop *evLoop, std::unique_ptr<Socket> socket);
    ~Connection();
    void recvMessage();
    void closeChannel();
    void errorChannel();
    void setCloseConnectionCallback(std::function<void(int)> func) { closeConnectionCallback_ = func; }
    void setRecvMessageCallback(std::function<void(shared_ptr_conn, std::string&)> func) { recvMessageCallback_ = func; }
    void setSentCompleteCallback(std::function<void(shared_ptr_conn)> func) { sentCompleteCallback_ = func; }
    void write(const std::string &message);   // 把处理过后的数据放到发送缓冲区
    void send();                       // 把发送缓冲区的消息发给客户端 
    void sendInEventLoop(const std::string &message);    // 在IO线程里把数据发送出去
    int fd() { return clientSocket_->fd(); }
private:
    EventLoop *evLoop_;
    std::unique_ptr<Socket>  clientSocket_;      
    std::unique_ptr<Channel> clientChannel_;    
    Buffer recvBuf_;            // 接收缓冲区
    Buffer sendBuf_;            // 发送缓冲区
    std::atomic_bool disconnect_{false};
    std::function<void(int)> closeConnectionCallback_;
    std::function<void(shared_ptr_conn, std::string&)> recvMessageCallback_;
    std::function<void(shared_ptr_conn)> sentCompleteCallback_;
};

#endif