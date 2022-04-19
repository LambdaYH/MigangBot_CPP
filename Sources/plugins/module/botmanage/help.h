#ifndef PLUGINS_MODULE_BOTMANAGE_HELP_H_
#define PLUGINS_MODULE_BOTMANAGE_HELP_H_

#include "plugins/plugin_interface.h"

namespace white
{

class Help : public PluginInterface
{
public:
    virtual void Register();
    void HelpMsg(const Event &event, onebot11::ApiBot &bot);
private:
    onebot11::ApiImpl api_impl;
};

inline void Help::Register()
{
    RegisterCommand(FULLMATCH, {".help", "/帮助"}, std::bind(&Help::HelpMsg, this, std::placeholders::_1, std::placeholders::_2));
}

inline void Help::HelpMsg( const Event &event, onebot11::ApiBot &bot)
{
    std::string msg = "欢迎使用米缸\n=============\n.dismiss 2215 使米缸退群\n请使用命令而不是直接T\n请不要随意禁言\n=============\n/help 查看FF14帮助\n-help 查看露儿帮助\nhelp 查看HoshinoBot帮助(仅群聊)\n=============\n.send 给维护者留言\n";
    msg += "=============\n[.help 其他] 查看其他功能\n[.help 设定] 查看设定\n(以上命令不包含[])\n(使用[.help]或[/帮助]可呼出本帮助)\n[默认开启FF14微博推送，如果不需要或嫌吵请使用[禁用 weibo-ff14]来关闭]";
    bot.send_msg(event, std::move(msg), true);
}

} // namespace white

#endif