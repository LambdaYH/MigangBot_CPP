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
    std::mutex mutex_;
    std::condition_variable cond_;
    bool is_close_;
    tbb::concurrent_queue<std::function<void()>> tasks_queue_;
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
    cond_.notify_all();
}

template<typename F>
inline void ThreadPool::AddTask(F &&task)
{
    {
        std::lock_guard<std::mutex> locker(mutex_);
        tasks_queue_.push(std::forward<F>(task));
    }
    cond_.notify_one();
}

inline void ThreadPool::Run(size_t thread_num)
{
    while(thread_num--)
    {
        std::thread{
        [&]{
                std::function<void()> task;
                std::unique_lock<std::mutex> locker(mutex_);
                while(!is_close_)
                {
                    if(tasks_queue_.empty())
                        cond_.wait(locker);
                    else
                    {
                        locker.unlock();
                        if(tasks_queue_.try_pop(task))
                        {
                            try
                            {
                                task();
                            }
                            catch(...)
                            {
                                // 异常处理
                                LOG_ERROR("Some Expection Happened...");
                            }
                        }
                        locker.lock();
                    }
                }
            }
        }.detach();
    }
}

} // namespace white

#endif