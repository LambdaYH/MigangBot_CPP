#pragma once

#include "modules/module_interface.h"

#include <unordered_set>

#include "message/message_segment.h"
#include "message/utility.h"
#include "modules/module/botmanage/config.h"

namespace white {
namespace module {

namespace plugin_botmanage_help {
constexpr auto kHelpConfigExample =
    "帮助信息:\n"
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

class Help : public Module {
 public:
  Help();
  virtual void Register() {
    OnPrefix({".help", "/帮助", "。help"}, make_pair("__help__", "机器人管理"), ACT_InClass(Help::HelpMsg),
             permission::NORMAL, permission::SUPERUSER);
  }

 private:
  void HelpMsg(const Event &event, onebot11::ApiBot &bot);

 private:
  Config config_;

  const std::string help_msg_group_others_;
  const std::string help_msg_friend_others_;
  std::unordered_set<std::string> custom_help_keyword_;
};

}  // namespace module
}  // namespace white