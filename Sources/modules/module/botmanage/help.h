#ifndef MIGANGBOTCPP_MODULES_MODULE_BOTMANAGE_HELP_H_
#define MIGANGBOTCPP_MODULES_MODULE_BOTMANAGE_HELP_H_

#include "modules/module_interface.h"

namespace white
{
namespace module
{

namespace plugin_botmanage_help
{
constexpr auto kHelpConfigExample = "help_msg_group: <群聊中显示的帮助信息>\n"
                                    "help_msg_friend: <私聊中显示的帮助信息>\n";
}

class Help : public Module
{
public:
    Help() : 
        Module("botmanage/help.yml", plugin_botmanage_help::kHelpConfigExample), 
        config_(LoadConfig()),
        help_msg_group_(config_["help_msg_group"].as<std::string>()),
        help_msg_friend_(config_["help_msg_friend"].as<std::string>())
    {}
    virtual void Register();
    void HelpMsg(const Event &event, onebot11::ApiBot &bot);
private:
    onebot11::ApiImpl api_impl;
    Config config_;

    const std::string help_msg_group_;
    const std::string help_msg_friend_;
};

inline void Help::Register()
{
    RegisterCommand(PREFIX, {".help", "/帮助", "。help"}, std::bind(&Help::HelpMsg, this, std::placeholders::_1, std::placeholders::_2));
}

inline void Help::HelpMsg(const Event &event, onebot11::ApiBot &bot)
{
    auto msg = event["message"].get<std::string>();
    auto text = ExtraPlainText(msg);
    Strip(text, ' ');
    if(text.size() == msg.size())
    {
        auto message_type = event["message_type"].get<std::string>();
        if(message_type[0] == 'g')
            bot.send_msg(event, help_msg_group_, true);
        else
            bot.send_msg(event, help_msg_friend_);
    }else if(text == "其他")
    {
        bot.send_msg(event, "还没写", true);
    }
}

} // namespace module
} // namespace white

#endif