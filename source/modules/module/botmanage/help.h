#ifndef MIGANGBOT_MODULES_MODULE_BOTMANAGE_HELP_H_
#define MIGANGBOT_MODULES_MODULE_BOTMANAGE_HELP_H_

#include "modules/module_interface.h"
#include "message/message_segment.h"
#include "message/utility.h"

#include <unordered_set>

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
                                    "  私聊: <私聊中显示>\n"
                                    "自定义:\n"
                                    "  关键词: [\"一个关键词\", \"第二个关键词\"]\n"
                                    "  一个关键词:\n"
                                    "    内容: <一个关键词的帮助信息>\n"
                                    "    图片格式: false # 是否以图片格式发送\n"
                                    "  第二个关键词:\n" 
                                    "    内容: <第二个关键词的帮助信息>\n"
                                    "    图片格式: true\n";
                                    
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
    {
        for(std::size_t i = 0; i < config_["自定义"]["关键词"].size(); ++i)
            custom_help_keyword_.insert(config_["自定义"]["关键词"][i].as<std::string>());
    }
    virtual void Register();
private:
    void HelpMsg(const Event &event, onebot11::ApiBot &bot);
    
private:
    Config config_;

    const std::string help_msg_group_;
    const std::string help_msg_friend_;
    const std::string help_msg_group_others_;
    const std::string help_msg_friend_others_;
    std::unordered_set<std::string> custom_help_keyword_;
};

inline void Help::Register()
{
    RegisterCommand(PREFIX, {".help", "/帮助", "。help"}, func(Help::HelpMsg));
}

inline void Help::HelpMsg(const Event &event, onebot11::ApiBot &bot)
{
    auto msg = message::ExtraPlainText(event);
    auto text = std::string_view(msg);
    message::Strip(text, ' ');
    auto message_type = event["message_type"].get<std::string>();
    if(text.size() == 0)
    {
        if(message_type[0] == 'g')
            bot.send_msg(event, help_msg_group_, true);
        else
            bot.send_msg(event, help_msg_friend_);
    }else if(text == "其他")
    {
        if(message_type[0] == 'g')
            bot.send_msg(event, message_segment::image(TextToImg(help_msg_group_others_)));
        else
            bot.send_msg(event, message_segment::image(TextToImg(help_msg_friend_others_)));
    }else if(custom_help_keyword_.count(std::string(text)))
    {
        auto content = config_["自定义"][std::string(text)]["内容"].as<std::string>();
        bool is_send_in_image = config_["自定义"][std::string(text)]["图片格式"].as<bool>();
        if(is_send_in_image)
            bot.send_msg(event, message_segment::image(TextToImg(content)));
        else
            bot.send_msg(event, content);
    }
}

} // namespace module
} // namespace white

#endif