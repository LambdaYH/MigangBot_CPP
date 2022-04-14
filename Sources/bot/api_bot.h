#ifndef MIGANGBOTCPP_BOT_API_BOT_H_
#define MIGANGBOTCPP_BOT_API_BOT_H_

#include <functional>
#include <future>
#include "message/type.h"
#include "api/onebot_11/api_impl.h"
#include "event/event.h"
#include <mutex>
#include <queue>

namespace white
{

class ApiBot
{

    friend class Bot;

public:
    ApiBot(std::function<void(const std::string &)> &notify) : notify_(notify) {}
    ~ApiBot(){}
    std::future<MsgId> send_msg(const Event &event, const std::string &message, bool auto_escape = false)
    {
        std::promise<MsgId> promise;
        notify_(api_impl_.send_msg(event, std::move(message)));
        auto ret = promise.get_future();
        AddToMsgidQueue(std::move(promise));
        return std::move(ret);
    }

private:
    template<typename T>
    void AddToMsgidQueue(T &&v)
    {
        std::lock_guard<std::mutex> locker(msgid_queue_mutex);
        msgid_queue_.push(std::forward<T>(v));
    }
private:
    std::mutex msgid_queue_mutex;
    std::queue<std::promise<MsgId>> msgid_queue_;
    onebot11::ApiImpl api_impl_;
    std::function<void(const std::string &)>& notify_;
};

} // namespace white

#endif