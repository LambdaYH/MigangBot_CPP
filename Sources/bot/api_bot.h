#ifndef MIGANGBOTCPP_BOT_API_BOT_H_
#define MIGANGBOTCPP_BOT_API_BOT_H_

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <random>
#include <chrono>

#include "type.h"
#include "api/onebot_11/api_impl.h"
#include "event/event.h"
#include "logger/logger.h"

namespace white
{
namespace onebot11
{

using Json = nlohmann::json;

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
                if constexpr (std::is_same<T, MsgId>::value)
                    return 0;
                else if constexpr (std::is_same<T, std::string>::value)
                    return "";
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

class ApiBot
{
public:
    FutureWrapper<MsgId>    send_private_msg(const Event &event, const uint64_t user_id, const std::string &message, bool auto_escape = false);
    FutureWrapper<MsgId>    send_group_msg(const Event &event, const uint64_t group_id, const std::string &message, bool at_sender = false, bool auto_escape = false);
    FutureWrapper<MsgId>    send_msg(const Event &event, const std::string &message, bool at_sender = false, bool auto_escape = false);
    void                    delete_msg(int32_t msg_id);

public:
    std::string WaitForNextMessage(const Event &event);

    std::string WaitForNextMessageFrom(QId person);

    bool IsSomeOneNeedMessage(QId user_id) const;

    bool IsNeedMessage(GId group_id, QId user_id) const;

    void FeedMessageTo(QId user_id, const std::string &message);

    void FeedMessage(GId group_id, QId user_id, const std::string &message);

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
    void EchoFunction(std::weak_ptr<std::promise<T>> weak_p, const Json &value)
    {
        if(value.is_null() || weak_p.expired())
            return;
        try
        {
            auto shared_p = weak_p.lock();
            if constexpr (std::is_same<T, MsgId>::value)
            {
                shared_p->set_value(value.value("message_id", 0));
            }
        }
        catch(const std::future_error& e)
        {
            std::cerr << "Exception In EchoFunction: " << e.what() << std::endl;
        }
        
    }

    template<typename T>
    FutureWrapper<T> Echo(Json &msg)
    {
        int echo_code = u_(random_engine_);
        msg["echo"] = echo_code;
        auto promise = std::make_shared<std::promise<T>>();
        auto func = std::bind(&ApiBot::EchoFunction<T>, this, std::weak_ptr(promise), std::placeholders::_1);
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

    std::unordered_map<GId, std::unordered_map<QId, std::queue<std::weak_ptr<std::promise<std::string>>>>> someone_group_message_;
    std::unordered_map<GId, std::unordered_map<QId, std::mutex>> someone_group_message_mutex_;

    std::unordered_map<QId, std::queue<std::weak_ptr<std::promise<std::string>>>> someone_need_message_;
    std::unordered_map<QId, std::mutex> someone_need_message_mutex_;
};

inline std::string ApiBot::WaitForNextMessage(const Event &event)
{
    std::shared_ptr<std::promise<std::string>> p = std::make_shared<std::promise<std::string>>();
    {
        QId user_id = event["user_id"].get<QId>();
        if(event.contains("group_id"))
        {
            GId group_id = event["group_id"].get<GId>();
            std::lock_guard<std::mutex> locker(someone_group_message_mutex_[group_id][user_id]);
            someone_group_message_[group_id][user_id].push(std::move(std::weak_ptr(p)));
        }else
        {
            std::lock_guard<std::mutex> locker(someone_need_message_mutex_[user_id]);
            someone_need_message_[user_id].push(std::move(std::weak_ptr(p)));
        }
    }
    FutureWrapper f{std::move(p)};
    return f.Ret();
}

inline std::string ApiBot::WaitForNextMessageFrom(QId person)
{
    std::shared_ptr<std::promise<std::string>> p = std::make_shared<std::promise<std::string>>();
    {
        std::lock_guard<std::mutex> locker(someone_need_message_mutex_[person]);
        someone_need_message_[person].push(std::move(std::weak_ptr(p)));
    }
    FutureWrapper f{std::move(p)};
    return f.Ret();
}

inline bool ApiBot::IsSomeOneNeedMessage(QId user_id) const 
{
    return someone_need_message_.count(user_id);
}

inline bool ApiBot::IsNeedMessage(GId group_id, QId user_id) const
{
    return someone_group_message_.count(group_id) && someone_group_message_.at(group_id).count(user_id);
}

inline void ApiBot::FeedMessageTo(QId user_id, const std::string &message)
{
    std::lock_guard<std::mutex> locker(someone_need_message_mutex_.at(user_id));
    while(!someone_need_message_.at(user_id).empty())
    {
        auto weak_p = someone_need_message_.at(user_id).front();
        someone_need_message_.at(user_id).pop();
        if(!weak_p.expired())
        {
            weak_p.lock()->set_value(message);
            break;
        }
    }
    if(someone_need_message_.at(user_id).empty())
        someone_need_message_.erase(user_id);
}   

inline void ApiBot::FeedMessage(GId group_id, QId user_id, const std::string &message)
{
    std::lock_guard<std::mutex> locker(someone_group_message_mutex_.at(group_id).at(user_id));
    while(!someone_group_message_.at(group_id).at(user_id).empty())
    {
        auto weak_p = someone_group_message_.at(group_id).at(user_id).front();
        someone_group_message_.at(group_id).at(user_id).pop();
        if(!weak_p.expired())
        {
            weak_p.lock()->set_value(message);
            break;
        }
    }
    if(someone_group_message_.at(group_id).at(user_id).empty())
        someone_group_message_.at(group_id).erase(user_id);
    if(someone_group_message_.at(group_id).empty())
        someone_group_message_.erase(group_id);
}

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
        msg = api_impl_.send_msg<false>(event.value("user_id", 0), event.value("group_id", 0), message, auto_escape);
    auto ret = Echo<MsgId>(msg);
    notify_(msg.dump());
    return ret;
}

inline void ApiBot::delete_msg(MsgId msg_id)
{
    Json msg = api_impl_.delete_msg(msg_id);
    notify_(msg.dump());
}

} // namespace onebot11
} // namespace white

#endif