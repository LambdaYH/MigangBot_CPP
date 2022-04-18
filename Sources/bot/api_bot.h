#ifndef MIGANGBOTCPP_BOT_API_BOT_H_
#define MIGANGBOTCPP_BOT_API_BOT_H_

#include <functional>
#include <future>
#include "type.h"
#include "api/onebot_11/api_impl.h"
#include "event/event.h"
#include <mutex>
#include <queue>
#include <random>
#include "logger/logger.h"
#include <chrono>

namespace white
{

using Json = nlohmann::json;

template<typename T>
class FutureWrapper
{
public:
    FutureWrapper(std::shared_ptr<std::promise<T>> &&p) : promise_ptr_(std::move(p)), future_(promise_ptr_->get_future()) {}

    T Ret()
    {
        auto status = future_.wait_for(std::chrono::seconds(30));
        switch(status)
        {
            case std::future_status::timeout:
            case std::future_status::deferred:
            {
                if constexpr (std::is_same<T, MsgId>::value)
                    return 0;
                else if constexpr (std::is_same<T, std::string>::value)
                    return "";
            }
            break;
            case std::future_status::ready:
                return future_.get();
        }
    }
private:
    std::shared_ptr<std::promise<T>> promise_ptr_;
    std::future<T> future_;
};

class ApiBot
{
public:
    FutureWrapper<MsgId>    send_private_msg(const Event &event, const uint64_t user_id, const std::string &message, bool auto_escape = false);
    FutureWrapper<MsgId>    send_group_msg(const Event &event, const uint64_t group_id, const std::string &message, bool at_sender = false, bool auto_escape = false);
    FutureWrapper<MsgId>    send_msg(const Event &event, const std::string &message, bool at_sender = false, bool auto_escape = false);
    void                    delete_msg(int32_t msg_id);

public:
    std::string WaitForNextMessage()
    {
        std::shared_ptr<std::promise<std::string>> p = std::make_shared<std::promise<std::string>>();
        auto weak_p = std::weak_ptr(p);
        FutureWrapper f{std::move(p)};
        {
            std::lock_guard<std::mutex> locker(wait_for_message_to_process_further_mutex_);
            wait_for_message_to_process_further_queue_.push(std::move(weak_p));
        }
        return f.Ret();
    }

    bool IsNeedMessage()
    {
        return !wait_for_message_to_process_further_queue_.empty();
    }

    void FeedMessage(const std::string &message)
    {
        std::lock_guard<std::mutex> locker(wait_for_message_to_process_further_mutex_);
        while(!wait_for_message_to_process_further_queue_.empty())
        {
            auto weak_p = wait_for_message_to_process_further_queue_.front();
            wait_for_message_to_process_further_queue_.pop();
            if(!weak_p.expired())
            {
                weak_p.lock()->set_value(message);
                break;
            }
        }
    }

public:
    ApiBot(std::function<void(const std::string &)> &notify, std::function<void(const int, std::function<void(const Json &)> &&)> &set_echo_function) : 
        notify_(notify),
        set_echo_function_(set_echo_function), 
        u_(1)
    {

    }

    ~ApiBot() {}

private:
    template<typename T>
    void EchoFuntion(std::weak_ptr<std::promise<T>> weak_p, const Json &value)
    {
        if(value.is_null() || weak_p.expired())
            return;
        auto shared_p = weak_p.lock();
        if constexpr (std::is_same<T, MsgId>::value)
        {
            shared_p->set_value(value.value("message_id", 0));
        }
    }

    template<typename T>
    FutureWrapper<T> Echo(Json &msg)
    {
        int echo_code = u_(random_engine_);
        msg["echo"] = echo_code;
        std::shared_ptr<std::promise<T>> promise = std::make_shared<std::promise<T>>();
        auto func = std::bind(&ApiBot::EchoFuntion<MsgId>, this, std::weak_ptr(promise), std::placeholders::_1);
        FutureWrapper<T> ret{std::move(promise)};
        set_echo_function_(echo_code, std::move(func));
        return ret;
    }
private:
    onebot11::ApiImpl api_impl_;
    std::function<void(const std::string &)> &notify_;
    std::function<void(const int, std::function<void(const Json &)> &&)> &set_echo_function_;
    std::mt19937 random_engine_;
    std::uniform_int_distribution<int> u_;

    std::mutex wait_for_message_to_process_further_mutex_;
    std::queue<std::weak_ptr<std::promise<std::string>>> wait_for_message_to_process_further_queue_;
};

inline FutureWrapper<MsgId> ApiBot::send_private_msg(const Event &event, const uint64_t user_id, const std::string &message, bool auto_escape)
{
    Json msg = api_impl_.send_private_msg(event.value("user_id", 0), message, event.value("group_id", 0), auto_escape);
    auto ret = Echo<MsgId>(msg);
    notify_(msg.dump());
    return ret;
}

inline FutureWrapper<MsgId> ApiBot::send_group_msg(const Event &event, const uint64_t group_id, const std::string &message, bool at_sender, bool auto_escape)
{
    Json msg = api_impl_.send_group_msg(event.value("group_id", 0), at_sender ? fmt::format("[CQ:at,qq={}]{}", event.value("user_id", 0), message) : message, auto_escape);
    auto ret = Echo<MsgId>(msg);
    notify_(msg.dump());
    return ret;
}

inline FutureWrapper<MsgId> ApiBot::send_msg(const Event &event, const std::string &message, bool at_sender, bool auto_escape)
{
    Json msg;
    if (event.value("message_type", "private") == "group")
        msg = api_impl_.send_msg<true>(event.value("user_id", 0), event.value("group_id", 0), at_sender ? fmt::format("[CQ:at,qq={}]{}", event.value("user_id", 0), message) : message, auto_escape);
    else
        msg = api_impl_.send_msg<false>(event.value("user_id", 0), event.value("group_id", 0), at_sender ? fmt::format("[CQ:at,qq={}]{}", event.value("user_id", 0), message) : message, auto_escape);
    auto ret = Echo<MsgId>(msg);
    notify_(msg.dump());
    return ret;
}

inline void ApiBot::delete_msg(MsgId msg_id)
{
    Json msg = api_impl_.delete_msg(msg_id);
    notify_(msg.dump());
}

} // namespace white

#endif