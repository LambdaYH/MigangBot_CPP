#ifndef MIGANGBOTCPP_BOT_RING_BUFFER_H_
#define MIGANGBOTCPP_BOT_RING_BUFFER_H_

#include <vector>
#include <atomic>

namespace white
{

// 在假设不会满出的情况下
template<typename T>
class RingBuffer
{
public:
    RingBuffer(std::size_t size) : 
        size_(size), 
        read_idx_(0), 
        write_idx_(0),
        buffer_(size_)
    {}

    bool empty()
    {
        return write_idx_ == read_idx_;
    }

    void push(const T &v)
    {
        buffer_[write_idx_] = v;
        if(write_idx_ + 1 >= size_)
            write_idx_ = 0;
        else
            ++write_idx_;
    }

    void push(const T &&v)
    {
        buffer_[write_idx_] = std::move(v); 
        if(write_idx_ + 1 >= size_)
            write_idx_ = 0;
        else
            ++write_idx_;
    }

    T pop()
    {
        auto ret = std::move(buffer_[read_idx_]);
        if(read_idx_ + 1 >= size_)
            read_idx_ = 0;
        else
            ++read_idx_;
        return ret;
    }

private:
    std::size_t size_;  
    std::atomic_size_t read_idx_;
    std::atomic_size_t write_idx_;
    std::vector<T> buffer_;
  
};

} // namespace white

#endif