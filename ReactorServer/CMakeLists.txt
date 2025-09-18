cmake_minimum_required(VERSION 3.20)
project(epoll_echo)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server 
    epollserver.cc 
    InetAddress.h 
    Socket.h 
    Epoll.cc 
    Channel.cc 
    EventLoop.cc
    Acceptor.cc
    Connection.cc
    TcpServer.cc 
    Buffer.cc
    EchoServer.cc
    ThreadPool.cc
)


add_executable(client client.cc)