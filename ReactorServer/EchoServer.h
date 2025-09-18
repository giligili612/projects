#ifndef ECHOSERVER_H
#define ECHOSERVER_H
#include "Connection.h"
#include "TcpServer.h"
#include "ThreadPool.h"
#include <cstddef>
class EchoServer {
public:
    EchoServer(const std::string& ip, uint16_t port, size_t thread_num, size_t work_threads_num = 5);
    void Start();
    void EchoMessage(shared_ptr_conn conn, std::string &message);
    void HandleBusiness(shared_ptr_conn conn, std::string &message);
    void newConnection(shared_ptr_conn conn);
    void closeConnection(shared_ptr_conn conn);
    void epollTimeout(shared_ptr_conn);
private:
    TcpServer server_base_;
    ThreadPool work_threads_;   // 工作线程，具体计算的业务交由线程池里的线程来处理
};

#endif