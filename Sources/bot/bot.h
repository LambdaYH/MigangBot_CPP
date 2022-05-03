#ifndef MIGANGBOTCPP_BOT_BOT_H_
#define MIGANGBOTCPP_BOT_BOT_H_

#include <hv/WebSocketServer.h>
#include <condition_variable>
#include <nlohmann/json.hpp>
#include <oneapi/tbb/concurrent_unordered_map.h>
#include <thread>
#include <mutex>
#include <string>
#include <utility>
#include <functional>
#include <queue>

#include "bot/onebot_11/api_bot.h"
#include "version.h"
#include "global_config.h"
#include "event/event.h"
#include "event/event_handler.h"

namespace white
{

using Json = nlohmann::json;

class Bot : public std::enable_shared_from_this<Bot>
{
public:
    explicit 
    Bot();
    ~Bot();

    void Run(const WebSocketChannelPtr& channel);

    void OnRead(const std::string &msg);

private:
    void OnRun();

    void Process(const std::string &message);

    void OnProcess(const std::string &message);

    void Notify(const std::string &msg);

    void SetEchoFunction(const std::time_t echo_code, std::function<void(const Json &)> &&func);

    bool EventProcess(const Event &event);

private:
    WebSocketChannelPtr channel_;
    std::queue<std::string> writable_msg_queue_;
    tbb::concurrent_unordered_map<std::time_t, std::function<void(const Json &)>> echo_function_;
    std::function<void(const std::string &)> notify_;
    std::function<void(const std::time_t, std::function<void(const Json &)> &&)> set_echo_function_;
    onebot11::ApiBot api_bot_;
    std::function<bool(Event &)> event_handler_;
    
};

inline Bot::Bot() :
        notify_(std::bind(&Bot::Notify, this, std::placeholders::_1)),
        set_echo_function_(std::bind(&Bot::SetEchoFunction, this, std::placeholders::_1, std::placeholders::_2)),
        api_bot_(notify_, set_echo_function_),
        event_handler_( std::bind(&EventHandler::Handle, &EventHandler::GetInstance(), std::placeholders::_1, std::ref(api_bot_)) )
{
    
}

inline Bot::~Bot()
{

}

inline void Bot::Run(const WebSocketChannelPtr& channel)
{
    channel_ = channel;
    OnRun();
}

inline void Bot::OnRun()
{
    for(auto superuser : config::SUPERUSERS)
        api_bot_.send_private_msg(superuser, fmt::format("MigangBot已启动\n版本: {}", kMigangBotVersion));
}

inline void Bot::OnRead(const std::string &msg)
{
    Process(msg);
}

inline void Bot::Notify(const std::string &msg)
{
    LOG_DEBUG("Msg To sent: {}", msg);
    channel_->send(msg);
}

inline void Bot::SetEchoFunction(const std::time_t echo_code, std::function<void(const Json &)> &&func)
{
    echo_function_[echo_code] = std::move(func);
}

inline void Bot::Process(const std::string &message)
{
    EventHandler::GetInstance().AddTask(std::bind(&Bot::OnProcess, this, message));
}

inline void Bot::OnProcess(const std::string &message)
{
    try
    {
        auto msg = nlohmann::json::parse(message);
        if(EventProcess(msg))
            event_handler_(msg);
    }catch(nlohmann::json::exception &e)
    {
        LOG_ERROR("Exception: {}", e.what());
    }
}

inline bool Bot::EventProcess(const Event &event)
{
    if(event.contains("retcode"))
    {
        std::time_t echo_code = 0;
        if (event.contains("echo"))
            echo_code = event["echo"].get<std::time_t>();
        if (echo_function_.count(echo_code))
        {
            echo_function_.at(echo_code)(event["data"]);
            echo_function_.unsafe_erase(echo_code);
        }
        return false;
    }else if(event.contains("message"))
    {
        QId user_id = event["user_id"].get<QId>();
        if(event.contains("group_id"))
        {
            GId group_id = event["group_id"].get<GId>();
            if(api_bot_.IsNeedMessage(group_id, user_id))
                api_bot_.FeedMessage(group_id, user_id, event["message"].get<std::string>());
        }
        if(api_bot_.IsSomeOneNeedMessage(user_id))
            api_bot_.FeedMessageTo(user_id, event["message"].get<std::string>());
    }
    return true;
}

} // namespace white

#endif