#ifndef MIGANGBOTCPP_BOT_ONEBOT_11_API_BOT_H_
#define MIGANGBOTCPP_BOT_ONEBOT_11_API_BOT_H_

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <random>
#include <chrono>
#include <bot/convert_to_string.h>

#include "type.h"
#include "api/onebot_11/api_impl.h"
#include "bot/onebot_11/future_wrapper.h"
#include "event/event.h"
#include "logger/logger.h"

namespace white
{
namespace onebot11
{

using Json = nlohmann::json;

std::time_t GetTimeStampMicro();

class ApiBot
{
public:
    template<typename T>
    FutureWrapper<MessageID>        send_private_msg(const QId user_id, T message, bool auto_escape = false);
    
    template<typename T>
    FutureWrapper<MessageID>        send_group_msg(const GId group_id, T message, bool auto_escape = false);
    
    template<typename T>
    FutureWrapper<MessageID>        send_msg(const Event &event, T message, bool at_sender = false, bool auto_escape = false);
    
    FutureWrapper<GroupInfo>        get_group_info(const GId group_id, bool no_cache = false);
    
    void                            delete_msg(MsgId msg_id);

public:
    std::string WaitForNextMessage(const Event &event);

    std::string WaitForNextMessageFrom(QId person);

    bool IsSomeOneNeedMessage(QId user_id) const;

    bool IsNeedMessage(GId group_id, QId user_id) const;

    void FeedMessageTo(QId user_id, const std::string &message);

    void FeedMessage(GId group_id, QId user_id, const std::string &message);

public:
    ApiBot(std::function<void(const std::string &)> &notify, std::function<void(const std::time_t, std::function<void(const Json &)> &&)> &set_echo_function) : 
        notify_(notify),
        set_echo_function_(set_echo_function), 
        u_(-10000, 10000)
    {

    }

    ~ApiBot() {}

private:
    template<typename T>
    void EchoFunction(std::weak_ptr<std::promise<T>> weak_p, const Json &value)
    {
        if(weak_p.expired())
            return;
        try
        {
            auto shared_p = weak_p.lock();
            if constexpr (std::is_same<T, MessageID>::value)
            {
                if(value.is_null())
                    shared_p->set_value({0});
                else
                    shared_p->set_value({value["message_id"].get<MsgId>()});
            }
            else if constexpr (std::is_same<T, GroupInfo>::value)
            {
                if(value.is_null())
                    shared_p->set_value({0, "", 0, 0});
                else
                    shared_p->set_value({value["group_id"].get<GId>(), 
                                        value["group_name"].get<std::string>(),
                                        value["member_count"].get<int>(),
                                        value["max_member_count"].get<int>()});
            }
        }
        catch(const std::future_error& e)
        {
            LOG_ERROR("Exception In EchoFunction: {}", e.what());
        }
        
    }

    template<typename T>
    FutureWrapper<T> Echo(Json &msg)
    {
        std::time_t echo_code = GetTimeStampMicro() + u_(random_engine_); // 用纳秒会导致接受的和发送的不一致，姑且用微秒吧
        msg["echo"] = echo_code; // 多线程下降低冲突的可能性，加个随机数
        auto promise = std::make_shared<std::promise<T>>();
        auto func = std::bind(&ApiBot::EchoFunction<T>, this, std::weak_ptr(promise), std::placeholders::_1);
        FutureWrapper<T> ret{std::move(promise)};
        set_echo_function_(echo_code, std::move(func));
        return ret;
    }
private:
    onebot11::ApiImpl api_impl_;
    std::function<void(const std::string &)> &notify_;
    std::function<void(const std::time_t, std::function<void(const Json &)> &&)> &set_echo_function_;
    std::mt19937 random_engine_;
    std::uniform_int_distribution<std::time_t> u_;

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
            someone_group_message_[group_id][user_id].push(std::weak_ptr(p));
        }else
        {
            std::lock_guard<std::mutex> locker(someone_need_message_mutex_[user_id]);
            someone_need_message_[user_id].push(std::weak_ptr(p));
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
        someone_need_message_[person].push(std::weak_ptr(p));
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

template<typename T>
inline FutureWrapper<MessageID> ApiBot::send_private_msg(const uint64_t user_id, T message, bool auto_escape)
{
    Json msg = api_impl_.send_private_msg(user_id, ConvertToString(message), auto_escape);
    auto ret = Echo<MessageID>(msg);
    notify_(msg.dump());
    return ret;
}

template<typename T>
inline FutureWrapper<MessageID> ApiBot::send_group_msg(const uint64_t group_id, T message, bool auto_escape)
{
    Json msg = api_impl_.send_group_msg(group_id, ConvertToString(message), auto_escape);
    auto ret = Echo<MessageID>(msg);
    notify_(msg.dump());
    return ret;
}

template<typename T>
inline FutureWrapper<MessageID> ApiBot::send_msg(const Event &event, T message, bool at_sender, bool auto_escape)
{
    Json msg;
    if (event.value("message_type", "private") == "group")
        msg = api_impl_.send_msg<true>(event.value("user_id", 0), event.value("group_id", 0), at_sender ? fmt::format("[CQ:at,qq={}]{}", event.value("user_id", 0), ConvertToString(message)) : ConvertToString(message), auto_escape);
    else
        msg = api_impl_.send_msg<false>(event.value("user_id", 0), event.value("group_id", 0), ConvertToString(message), auto_escape);
    auto ret = Echo<MessageID>(msg);
    notify_(msg.dump());
    return ret;
}

inline FutureWrapper<GroupInfo> ApiBot::get_group_info(const GId group_id, bool no_cache)
{
    Json msg = api_impl_.get_group_info(group_id, no_cache);
    auto ret = Echo<GroupInfo>(msg);
    notify_(msg.dump());
    return ret;
}

inline void ApiBot::delete_msg(MsgId msg_id)
{
    Json msg = api_impl_.delete_msg(msg_id);
    notify_(msg.dump());
}

inline std::time_t GetTimeStampMicro()
{
    auto tp = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now());
    return tp.time_since_epoch().count();
}

} // namespace onebot11
} // namespace white

#endif