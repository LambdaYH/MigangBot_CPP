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
    Echo() : PluginInterface("") {}
    virtual void Register();

    void DoEcho(const Event &event, onebot11::ApiBot &bot);
private:
    onebot11::ApiImpl api_impl;
};

inline void Echo::Register()
{
    RegisterCommand(PREFIX, {"/echo", "/回声"}, std::bind(&Echo::DoEcho, this, std::placeholders::_1, std::placeholders::_2));
}

inline void Echo::DoEcho(const Event &event, onebot11::ApiBot &bot)
{
    std::string msg = event["message"].get<std::string>().substr(6);
    auto text = ExtraPlainText(msg);
    auto ret = bot.send_msg(event, std::string(text)).Ret();
    LOG_DEBUG("DoEcho: ret msgid: {}", ret);
}

} // namespace white

#endif