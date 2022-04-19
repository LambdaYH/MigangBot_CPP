#ifndef PLUGINS_MODULE_ECHO_H_
#define PLUGINS_MODULE_ECHO_H_

#include "plugins/plugin_interface.h"
#include "api/onebot_11/api_impl.h"
#include "event/event.h"
#include <functional>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <string_view>

namespace white
{

class Echo : public PluginInterface
{
public:
    virtual void Register(EventHandler &event_handler);
    void DoEcho(const Event &event, onebot11::ApiBot &bot);
private:
    onebot11::ApiImpl api_impl;
};

inline void Echo::Register(EventHandler &event_handler)
{
    event_handler.RegisterCommand(PREFIX, "/echo", std::bind(&Echo::DoEcho, this, std::placeholders::_1, std::placeholders::_2));
}

inline void Echo::DoEcho(const Event &event, onebot11::ApiBot &bot)
{
    std::string msg = event["message"].get<std::string>().substr(6);
    auto ret = bot.send_msg(event, std::string(msg)).Ret();
    LOG_DEBUG("ret msgid: {}", ret);
    auto new_msg = bot.WaitForNextMessageFrom(event["user_id"].get<QId>());
    ret = bot.send_msg(event, new_msg).Ret();
    bot.send_msg(event, std::to_string(ret));
}

} // namespace white

#endif