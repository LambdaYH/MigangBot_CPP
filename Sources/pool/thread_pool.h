/*
 * @Author       : mark
 * @Date         : 2020-06-25
 * @copyleft Apache 2.0
 */ 
#ifndef MIGANGBOTCPP_POOL_THREADPOOL_H_
#define MIGANGBOTCPP_POOL_THREADPOOL_H_

#include <mutex>
#include <condition_variable>
#include <oneapi/tbb/concurrent_queue.h>
#include <thread>
#include <functional>
#include <vector>
#include "logger/logger.h"

namespace white {
    
// unique_lock vs mutex : https://stackoverflow.com/questions/37945859/mutex-lock-vs-unique-lock

class ThreadPool
{
public:
    ThreadPool(std::size_t thread_num = 8);
    ~ThreadPool();

    template<typename F> // template for std::forward
    void AddTask(F &&task);

private:
    void Run(std::size_t thread_num);

private:
    bool is_close_;
    std::vector<std::thread> threads_;
    tbb::concurrent_bounded_queue<std::function<void()>> tasks_queue_;
};

inline ThreadPool::ThreadPool(std::size_t thread_num) :
is_close_(false)
{
    if(thread_num > 32)
        thread_num = 32;
    Run(thread_num);
}

inline ThreadPool::~ThreadPool()
{
    is_close_ = true;
    for(std::size_t i = 0; i < threads_.size(); ++i)
        tasks_queue_.push(std::function<void()>());
    for(auto &thread : threads_)
        thread.join();
}

template<typename F>
inline void ThreadPool::AddTask(F &&task)
{
    tasks_queue_.push(std::forward<F>(task));
}

inline void ThreadPool::Run(size_t thread_num)
{
    while(thread_num--)
    {
        threads_.push_back(
            std::thread{
            [&]{
                std::function<void()> task;
                while(true)
                {
                    tasks_queue_.pop(task);
                    if(is_close_)
                        return;
                    try
                    {
                        task();
                    }
                    catch(std::exception &e)
                    {
                        // 异常处理
                        LOG_ERROR("Some Expection Happened...: {}", e.what());
                    }
                }
            }
        });
    }
}

} // namespace white

#endif