#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <string>
#include <sys/types.h>

class Buffer
{
public:
    Buffer() = default;
    ~Buffer() = default;
    // 返回缓冲区首地址
    const char *data() { return buffer_.data(); }
    // 清空缓冲区
    void clear() { buffer_.clear(); }
    
    ssize_t size() { return buffer_.size(); }

    // 向缓冲区里写数据
    void append(const char *data, size_t size);
    // 清除缓冲区从pos开始指定num字节的数据
    void erase(size_t pos , size_t num);
private:
    std::string buffer_;
};

#endif