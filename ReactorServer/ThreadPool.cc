#include "ThreadPool.h"
#include <cstddef>
#include <cstdio>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <sys/syscall.h>

ThreadPool::ThreadPool(size_t size):stop_(false) {
    for(size_t i = 0; i < size; i++) {
        threads_.emplace_back([this](){
            // printf("create thread(%ld).\n", syscall(SYS_gettid));

            while(stop_ == false) {
                std::function<void()> task;     // 从任务队列里拿出来的任务

                {
                    std::unique_lock<std::mutex> lock(mux_); // 上锁
                    // 等待生产者的条件变量
                    cond_.wait(lock, [this](){
                        return (stop_ == true) || (task_queue_.empty() == false); //线程池停止或者任务队列非空则唤醒
                    });
                    // 线程池停止前先执行完任务队列内的所有任务
                    if((stop_ == true) && (task_queue_.empty() == true)) return;
                    // 从任务队列里出队一个任务
                    task = std::move(task_queue_.front());
                    task_queue_.pop();
                }
                
                // 执行任务
                task();
            }
        });
    }
}

void ThreadPool::addTask(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(mux_);
        task_queue_.push(task);
    }
    cond_.notify_one();
}

void ThreadPool::stop() {
    if(stop_) return;  // 防止重复调用

    stop_ = true;
    cond_.notify_all();

    for(auto &th : threads_) {
        if(th.joinable())
            th.join();
    }
}

ThreadPool::~ThreadPool() {
    if(!stop_)
        stop();
}