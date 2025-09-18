#ifndef INETADDRESS_H
#define INETADDRESS_H

#include <cstdint>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string_view>
#include <string>

class InetAddress
{
public:
    // ! inet_pton没有根据返回值安全检查
    explicit InetAddress(std::string_view ip, uint16_t port) {
        std::memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        inet_pton(AF_INET, std::string(ip).c_str(), &addr_.sin_addr);
    }

    explicit InetAddress(const std::string& ip, uint16_t port)
        : InetAddress(std::string_view(ip), port) {}

    explicit InetAddress(const std::string& ip, const std::string& port)
        : InetAddress(std::string_view(ip), parse_port(port)) {}

    explicit InetAddress(const char *ip, uint16_t port)
        : InetAddress(std::string_view(ip), port) {}

    explicit InetAddress(const char *ip, const char* port)
        : InetAddress(std::string_view(ip), parse_port(std::string(port))) {}

    explicit InetAddress(const sockaddr_in addr):addr_(addr) {}

    ~InetAddress() {}

    // TODO： 提供ip方法的线程安全版本
    const char*     ip()   const { return inet_ntoa(addr_.sin_addr); } 
    uint16_t        port() const { return ntohs(addr_.sin_port); }
    const sockaddr* addr() const { return (sockaddr*)&addr_; }
    void setAddr(sockaddr_in addr) { addr_ = addr; }
private:
    // ! 没有对port的范围检查：(port < 0 || port > 65535)
    static uint16_t parse_port(const std::string& port_str) {
        int port = std::stoi(port_str);
        return static_cast<uint16_t>(port);
    }
    sockaddr_in addr_{};
};

#endif