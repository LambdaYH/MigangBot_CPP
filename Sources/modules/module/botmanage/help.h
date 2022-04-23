#ifndef MIGANGBOTCPP_MODULES_MODULE_BOTMANAGE_HELP_H_
#define MIGANGBOTCPP_MODULES_MODULE_BOTMANAGE_HELP_H_

#include "modules/module_interface.h"
#include "message/message_segmentation.h"
#include "message/utility.h"

namespace white
{
namespace module
{

namespace plugin_botmanage_help
{
constexpr auto kHelpConfigExample = "帮助信息:\n"
                                    "  群聊: <群聊中显示>\n"
                                    "  私聊: <私聊中显示>\n"
                                    "其他帮助信息:\n"
                                    "  群聊: <群聊中显示>\n"
                                    "  私聊: <私聊中显示>\n";
                                    
}

class Help : public Module
{
public:
    Help() : 
        Module("botmanage/help.yml", plugin_botmanage_help::kHelpConfigExample), 
        config_(LoadConfig()),
        help_msg_group_(config_["帮助信息"]["群聊"].as<std::string>()),
        help_msg_friend_(config_["帮助信息"]["私聊"].as<std::string>()),
        help_msg_group_others_(config_["其他帮助信息"]["群聊"].as<std::string>()),
        help_msg_friend_others_(config_["其他帮助信息"]["私聊"].as<std::string>())
    {}
    virtual void Register();
    void HelpMsg(const Event &event, onebot11::ApiBot &bot);
private:
    onebot11::ApiImpl api_impl;
    Config config_;

    const std::string help_msg_group_;
    const std::string help_msg_friend_;
    const std::string help_msg_group_others_;
    const std::string help_msg_friend_others_;
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
    auto message_type = event["message_type"].get<std::string>();
    if(text.size() == msg.size())
    {
        if(message_type[0] == 'g')
            bot.send_msg(event, help_msg_group_, true);
        else
            bot.send_msg(event, help_msg_friend_);
    }else if(text == "其他")
    {
        if(message_type[0] == 'g')
            bot.send_msg(event, message_segmentation::image(TextToImg(help_msg_group_others_)));
        else
            bot.send_msg(event, message_segmentation::image(TextToImg(help_msg_friend_others_)));
    }
}

} // namespace module
} // namespace white

#endif