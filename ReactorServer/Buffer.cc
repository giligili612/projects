#include "Buffer.h"

// 向缓冲区里写数据
void Buffer::append(const char *data, size_t size)
{
    buffer_.append(data, size);
}

// 清除缓冲区从pos开始指定num字节的数据
void Buffer::erase(size_t pos , size_t num)
{
    buffer_.erase(pos, num);
}