#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <queue>
#include <sys/types.h>
#include <thread>
#include <vector>
#include <mutex>

class ThreadPool {
public:
    ThreadPool(size_t size);
    void addTask(std::function<void()> task);
    void stop();
    ~ThreadPool();
    size_t size() { return threads_.size(); }
private:
    std::vector<std::thread> threads_;       // 线程池里的线程
    std::queue<std::function<void()>> task_queue_;    // 任务队列
    std::mutex mux_;                         // 任务队列同步的互斥锁
    std::condition_variable cond_;          // 任务队列同步的信号量
    bool stop_;                              // 标记线程池是否在运行
};

#endif