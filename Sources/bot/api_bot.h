#ifndef MIGANGBOTCPP_BOT_API_BOT_H_
#define MIGANGBOTCPP_BOT_API_BOT_H_

#include <functional>
#include <future>
#include "message/type.h"
#include "api/onebot_11/api_impl.h"
#include "event/event.h"
#include <mutex>
#include <queue>
#include <random>
#include "logger/logger.h"
namespace white
{

using Json = nlohmann::json;

class ApiBot
{
public:
    ApiBot(std::function<void(const std::string &)> &notify, std::function<void(const int, std::function<void(const Json &)> &&)> &set_echo_function) : 
        notify_(notify),
        set_echo_function_(set_echo_function), 
        u_(1)
    {

    }

    ~ApiBot() {}
    std::future<MsgId> send_msg(const Event &event, const std::string &message, bool auto_escape = false)
    {
        Json msg = api_impl_.send_msg(event, message);
        auto ret = Echo<MsgId>(msg);
        notify_(msg.dump());
        return ret;
    }
private:
    template<typename T>
    void EchoFuntion(std::shared_ptr<std::promise<T>> p, const Json &value)
    {
        if constexpr (std::is_same<T, MsgId>::value)
        {
            p->set_value(value["message_id"].get<int32_t>());
        }
    }

    template<typename T>
    std::future<T> Echo(Json &msg)
    {
        int echo_code = u_(random_engine_);
        msg["echo"] = echo_code;
        std::shared_ptr<std::promise<T>> promise = std::make_shared<std::promise<T>>();
        std::future<T> ret = promise->get_future();
        auto func = std::bind(&ApiBot::EchoFuntion<MsgId>, this, promise, std::placeholders::_1);
        set_echo_function_(echo_code, std::move(func));
        return ret;
    }
private:
    onebot11::ApiImpl api_impl_;
    std::function<void(const std::string &)> &notify_;
    std::function<void(const int, std::function<void(const Json &)> &&)> &set_echo_function_;
    std::mt19937 random_engine_;
    std::uniform_int_distribution<int> u_;
};

} // namespace white

#endif