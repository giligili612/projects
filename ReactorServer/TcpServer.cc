#include "TcpServer.h"
#include "Connection.h"
#include "EventLoop.h"
#include <cstdio>
#include <functional>
#include <memory>
#include <mutex>

TcpServer::TcpServer(const std::string& ip, uint16_t port, size_t thread_num): thread_poll_(thread_num),
        thread_num_(thread_num), mainLoop_(new EventLoop()),acceptor_(new Acceptor(mainLoop_.get(), ip, port))
{
    // 主事件循环，用于接收连接
    
    mainLoop_->setTimeOutCallback(std::bind(&TcpServer::epollTimeout, this, std::placeholders::_1));
    
    // 从事件循环，用于处理具体连接
    for(int i = 0; i < thread_num_; i++) {
        std::unique_ptr<EventLoop> loop(new EventLoop());
        loop->setTimeOutCallback(std::bind(&TcpServer::epollTimeout, this, std::placeholders::_1));
        // loop创建完并设置好之后再加入到里面
        subLoops_.emplace_back(std::move(loop));
    }
    // 所有事件循环都准备好后再往线程池里加
    for(int i = 0; i < thread_num_; i++) {
        thread_poll_.addTask(std::bind(&EventLoop::run, subLoops_[i].get()));
    }   

    //acceptor_ = new Acceptor(mainLoop_.get(), ip, port); // 在Acceptor里初始化了mainLoop_
    acceptor_->setNewConnectionSlot(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
    
}

TcpServer::~TcpServer()
{  
    thread_poll_.stop();
}

void TcpServer::start()
{
    mainLoop_->run();
}

void TcpServer::newConnection(std::unique_ptr<Socket>socket)
{
    printf("accept client(fd=%d, ip=%s, port=%d)\n", socket->fd(), socket->ip().c_str(), socket->port());
    shared_ptr_conn conn = std::make_shared<Connection>(subLoops_[socket->fd() % thread_num_].get(), std::move(socket));
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connections_[conn->fd()] = conn; 
    }
     
    conn->setCloseConnectionCallback(std::bind(&TcpServer::closeConnection, this, std::placeholders::_1));
    conn->setRecvMessageCallback(std::bind(&TcpServer::HandleData, this, std::placeholders::_1, std::placeholders::_2));
    conn->setSentCompleteCallback(std::bind(&TcpServer::sendComplete, this, std::placeholders::_1));
    if(conn) newConnectionCallback_(conn);
}
// 清理连接表
void TcpServer::closeConnection(int fd)
{
    shared_ptr_conn conn;
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        auto it = connections_.find(fd);
        if(it != connections_.end()) {
            conn = it->second;
            connections_.erase(it);
        }
    }
    
    if(conn && closeConnectionCallback_) {
        closeConnectionCallback_(conn);
    }
    
}

void TcpServer::HandleData(shared_ptr_conn conn, std::string &message)
{
    handleDataCallback_(conn, message);
}

void TcpServer::sendComplete(shared_ptr_conn conn)
{
    // puts("The message sent completed.\n");
}

void TcpServer::epollTimeout(EventLoop* eventloop) {
    // printf("Epoll(fd=%d) loop timeout:\n", eventloop->epoll()->fd());
}