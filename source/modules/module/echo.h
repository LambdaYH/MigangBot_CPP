#ifndef MIGANGBOT_MODULES_MODULE_ECHO_H_
#define MIGANGBOT_MODULES_MODULE_ECHO_H_

#include "co/co.h"
#include "modules/module_interface.h"

#include <string_view>

namespace white
{
namespace module
{

class Echo : public Module
{
public:
    Echo() : Module() {}
    virtual void Register();

private:
    void DoEcho(const Event &event, onebot11::ApiBot &bot);
};

inline void Echo::Register()
{
    RegisterCommand(PREFIX, {"/echo", "/回声"}, ACT(Echo::DoEcho), permission::NORMAL);
}

inline void Echo::DoEcho(const Event &event, onebot11::ApiBot &bot)
{
    auto text = message::ExtraPlainText(event);
    auto ret = bot.send_msg(event, text).Ret();
    bot.send_msg(event, bot.WaitForNextMessage(event));
}

} // namespace module
} // namespace white

#endif