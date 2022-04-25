#ifndef MIGANGBOTCPP_MODULES_MODULE_ECHO_H_
#define MIGANGBOTCPP_MODULES_MODULE_ECHO_H_

#include "modules/module_interface.h"
#include <functional>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <string_view>

namespace white
{
namespace module
{

class Echo : public Module
{
public:
    Echo() : Module("") {}
    virtual void Register();

    void DoEcho(const Event &event, onebot11::ApiBot &bot);
};

inline void Echo::Register()
{
    RegisterCommand(PREFIX, {"/echo", "/回声"}, func(Echo::DoEcho), permission::SUPERUSER);
    RegisterCommand(PREFIX, {"/e"}, func(Echo::DoEcho), permission::NORMAL, true);
}

inline void Echo::DoEcho(const Event &event, onebot11::ApiBot &bot)
{
    std::string msg = event["message"].get<std::string>();
    auto text = ExtraPlainText(msg);
    auto ret = bot.send_msg(event, std::string(text)).Ret();
    bot.send_msg(event, std::to_string(ret));
    LOG_DEBUG("DoEcho: ret msgid: {}", ret);
}

} // namespace plugins
} // namespace white

#endif