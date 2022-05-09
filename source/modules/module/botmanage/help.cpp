#include "modules/module/botmanage/help.h"

namespace white {
namespace module {

Help::Help()
    : Module("botmanage/help.yml", plugin_botmanage_help::kHelpConfigExample),
      config_(LoadConfig()),
      help_msg_group_others_(config_["其他帮助信息"]["群聊"].as<std::string>()),
      help_msg_friend_others_(
          config_["其他帮助信息"]["私聊"].as<std::string>()) {
  botmanage::help_msg_group_ = config_["帮助信息"]["群聊"].as<std::string>();
  botmanage::help_msg_friend_ = config_["帮助信息"]["私聊"].as<std::string>();
  for (std::size_t i = 0; i < config_["自定义"]["关键词"].size(); ++i)
    custom_help_keyword_.insert(
        config_["自定义"]["关键词"][i].as<std::string>());
}

void Help::HelpMsg(const Event &event, onebot11::ApiBot &bot) {
  auto msg = message::ExtraPlainText(event);
  auto text = std::string_view(msg);
  message::Strip(text, ' ');
  auto message_type = event["message_type"].get<std::string>();
  if (text.size() == 0) {
    if (message_type[0] == 'g')
      bot.send_msg(event, botmanage::help_msg_group_, true);
    else
      bot.send_msg(event, botmanage::help_msg_friend_);
  } else if (text == "其他") {
    if (message_type[0] == 'g')
      bot.send_msg(event,
                   message_segment::image(TextToImg(help_msg_group_others_)));
    else
      bot.send_msg(event,
                   message_segment::image(TextToImg(help_msg_friend_others_)));
  } else if (custom_help_keyword_.count(std::string(text))) {
    auto content =
        config_["自定义"][std::string(text)]["内容"].as<std::string>();
    bool is_send_in_image =
        config_["自定义"][std::string(text)]["图片格式"].as<bool>();
    if (is_send_in_image)
      bot.send_msg(event, message_segment::image(TextToImg(content)));
    else
      bot.send_msg(event, content);
  }
}

}  // namespace module
}  // namespace white