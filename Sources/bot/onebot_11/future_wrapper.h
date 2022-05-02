#ifndef MIGANGBOTCPP_BOT_ONEBOT_11_FUTURE_WRAPPER_H_
#define MIGANGBOTCPP_BOT_ONEBOT_11_FUTURE_WRAPPER_H_

#include <future>
#include "type.h"

namespace white
{
namespace onebot11
{
    
template<typename T>
class FutureWrapper
{
public:
    FutureWrapper(std::shared_ptr<std::promise<T>> &&p) : promise_(std::move(p)), future_(promise_->get_future()) {}

    T Ret()
    {
        auto status = future_.wait_for(std::chrono::seconds(30));
        switch(status)
        {
            case std::future_status::timeout:
            case std::future_status::deferred:
            {
                if constexpr (std::is_same<T, MessageID>::value)
                    return {0};
                else if constexpr (std::is_same<T, std::string>::value)
                    return "";
                else if constexpr (std::is_same<T, GroupInfo>::value)
                    return {0, "", 0, 0};
            }
            break;
            case std::future_status::ready:
                break;
        }
        return future_.get();
    }
private:
    std::shared_ptr<std::promise<T>> promise_;
    std::future<T> future_;
};

} // namespace onebot11
} // namespace white
#endif