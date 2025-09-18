#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "Connection.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "Socket.h"
#include "ThreadPool.h"
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
class TcpServer
{
public:
    TcpServer(const std::string& ip, uint16_t port, size_t thread_num = 3);
    ~TcpServer();

    // 接口，供业务类指定
    void setNewConnectionCallback(std::function<void(shared_ptr_conn conn)> fun) { newConnectionCallback_ = fun; }
    void setCloseConnectionCallback(std::function<void(shared_ptr_conn conn)> fun) { closeConnectionCallback_ = fun; }
    void setEpollTimeoutCallback(std::function<void(shared_ptr_conn conn)> fun) { epollTimeoutCallback_ = fun; }
    void setHandleDataCallback(std::function<void(shared_ptr_conn conn, std::string &message)> fun) {
        handleDataCallback_ = fun;
    }
    // 底层逻辑
    void start();
    void newConnection(std::unique_ptr<Socket> socket);     // 创建Connection,接受来自Acceptor的信号
    void closeConnection(int fd);               // 关闭Connection
    void HandleData(shared_ptr_conn conn, std::string&);         // 发送消息由TcpServer来管理，如果不止回显功能的话，会用另外独立的处理业务的类
    void sendComplete(shared_ptr_conn conn);        // 发送消息完成
    void epollTimeout(EventLoop* eventloop);    // epoll超时
private:
    std::unique_ptr<EventLoop> mainLoop_;                // 主事件循环
    std::vector<std::unique_ptr<EventLoop>> subLoops_;   // 从事件循环
    ThreadPool thread_poll_;             // 线程池
    size_t thread_num_;                  // 线程池大小
    std::unique_ptr<Acceptor> acceptor_;    // 一个TcpServer只有一个Acceptor
    
    std::mutex connections_mutex_;
    std::map<int,shared_ptr_conn> connections_;  // 管理所有的Connection
    
    std::function<void(shared_ptr_conn conn)> newConnectionCallback_;
    std::function<void(shared_ptr_conn conn)> closeConnectionCallback_;
    std::function<void(shared_ptr_conn conn)> epollTimeoutCallback_;
    std::function<void(shared_ptr_conn conn, std::string& message)> handleDataCallback_;
};

#endif