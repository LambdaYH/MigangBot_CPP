#ifndef MIGANGBOT_CO_FUTURE_H_
#define MIGANGBOT_CO_FUTURE_H_

#include <co/co.h>

namespace white
{

template<typename T> class co_future;
template<typename T> class co_promise;

enum class co_future_status
{
    ready,
    timeout
};

template<typename T>
class co_future
{

friend class co_promise<T>;

public:
    T get()
    {
        if(!is_complete_)
            event_.wait();
        return std::move(value_);
    }

    void wait()
    {
        if(!is_complete_)
            event_.wait();
    }


    co_future_status wait_for(uint32 ms)
    {
        if(is_complete_)
            return co_future_status::ready;
        if(event_.wait(ms))
            return co_future_status::ready;
        return co_future_status::timeout;
    }

private:
    co_future(T &value, co::Event &event, bool &is_complete) : value_(value), event_(event), is_complete_(is_complete) {}

private:
    co::Event &event_;
    T &value_;
    bool &is_complete_;

};

template<typename T>
class co_promise
{
public:
    co_promise() : is_complete_(false) {}
    ~co_promise() {}

    co_promise(const co_promise &) = delete;
    co_promise &operator=(const co_promise &) = delete;

    co_promise(const co_promise &&p)
    {
        this->event_ = std::move(p.event_);
        this->value_ = std::move(p.value_);
        this->is_complete_ = std::move(is_complete_);
    }

    co_promise &&operator=(const co_promise &&p)
    {
        this->event_ = std::move(p.event_);
        this->value_ = std::move(p.value_);
        this->is_complete_ = std::move(is_complete_);
    }

public:
    co_future<T> get_future()
    {
        return co_future<T>(value_, event_, is_complete_);
    }
    
    void set_value(const T& value)
    {
        value_ = value;
        is_complete_ = true;
        event_.signal();
    }

    void set_value(T& value)
    {
        value_ = value;
        is_complete_ = true;
        event_.signal();
    }

    void set_value(T&& value)
    {
        value_ = std::move(value);
        is_complete_ = true;
        event_.signal();
    }

    void set_value()
    {
        is_complete_ = true;
        event_.signal();
    }

private:
    co::Event event_;
    T value_;
    bool is_complete_;
};

} // namespace white

#endif