#include "EchoServer.h"
#include <cstdio>
#include <ctime>
#include <functional>

EchoServer::EchoServer(const std::string& ip, uint16_t port, size_t thread_num, size_t work_threads_num):
    server_base_(ip, port, thread_num), work_threads_(work_threads_num) {
    server_base_.setNewConnectionCallback(std::bind(&EchoServer::newConnection, this, std::placeholders::_1));
    server_base_.setCloseConnectionCallback(std::bind(&EchoServer::closeConnection, this, std::placeholders::_1));
    server_base_.setEpollTimeoutCallback(std::bind(&EchoServer::epollTimeout, this, std::placeholders::_1));
    server_base_.setHandleDataCallback(std::bind(&EchoServer::HandleBusiness, this, std::placeholders::_1, std::placeholders::_2));
}

void EchoServer::Start() {
    server_base_.start();
}

void EchoServer::newConnection(shared_ptr_conn conn) {
    printf("EchoServer new connection fd = %d at %ld.\n", conn->fd(), time(0));
}

void EchoServer::closeConnection(shared_ptr_conn conn) {
    printf("EchoServer close connection fd = %d at %ld.\n", conn->fd(), time(0));
}

void EchoServer::epollTimeout(shared_ptr_conn conn) {
    // printf("EchoServer(fd=%d) epoll timeout.\n", conn->fd());
}

void EchoServer::HandleBusiness(shared_ptr_conn conn, std::string &message) {
    if(work_threads_.size() == 0)
        EchoMessage(conn, message);
    else
        work_threads_.addTask(std::bind(&EchoServer::EchoMessage, this, conn, message));
}

void EchoServer::EchoMessage(shared_ptr_conn conn, std::string &message) {
    // 这里处理收到的数据
    message = "reply:" + message;
    // 写到connction的输出缓冲区里去
    conn->write(message);
}