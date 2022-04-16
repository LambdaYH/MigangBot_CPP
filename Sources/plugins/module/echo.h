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
    void DoEcho(const Event &event, ApiBot &bot);
private:
    onebot11::ApiImpl api_impl;
};

inline void Echo::Register(EventHandler &event_handler)
{
    event_handler.RegisterCommand(PREFIX, "/echo", std::bind(&Echo::DoEcho, this, std::placeholders::_1, std::placeholders::_2));
}

inline void Echo::DoEcho( const Event &event, ApiBot &bot)
{
    std::string msg = event["message"].get<std::string>().substr(6);
    auto ret = bot.send_msg(event, std::string(msg)).Ret();
    LOG_DEBUG("ret msgid: {}", ret);
}

} // namespace white

#endif