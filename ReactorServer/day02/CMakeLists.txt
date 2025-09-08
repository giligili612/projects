cmake_minimum_required(VERSION 3.20)
project(epoll_echo)

add_executable(server epollserver.cc InetAddress.hpp Socket.hpp Epoll.hpp)
add_executable(client client.cc)