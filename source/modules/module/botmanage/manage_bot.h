#pragma once

#include <string_view>

#include "event/Registrar.h"
#include "fmt/format.h"
#include "message/utility.h"
#include "modules/module_interface.h"
#include "modules/module/botmanage/config.h"
#include "permission/permission.h"
#include "utility.h"

namespace white {
namespace module {

class ManageBot : public Module {
 public:
  ManageBot() : Module("botmanage/botmanage.yml", botmanage::kConfigExample) {}
  virtual void Register() override {
    OnPrefix({"migangbot"}, make_pair("__manage_bot__", "机器人管理"),
             "管理机器人各项...", ACT_InClass(ManageBot::Handle),
             permission::SUPERUSER, permission::ALWAYS_ON, true, false);
  }

 private:
  void Handle(const Event &event, onebot11::ApiBot &bot);
};

inline void ManageBot::Handle(const Event &event, onebot11::ApiBot &bot) {
  static const auto hint =
      "Usage:\n \
-l   退出群聊\n \
-blq 列举黑名单qq\n \
-blg 列举黑名单群\n \
-caf 改变允许加好友状态\n \
-cag 改变允许入群状态\n \
-gf  禁止一切群邀请(严格)\n";
  std::string_view option{message::Strip(message::ExtraPlainText(event))};
  if (option.empty()) {
    bot.send(event, hint);
  } else {
    if (option.starts_with("-l")) {
      option.remove_prefix(
          std::min(option.size(), option.find_first_not_of(' ', 2)));
      if (!IsDigitStr(option)) {
        bot.send(event, "请输入合法群号");
        return;
      }
      bot.set_group_leave(std::atoll(option.data()));
      bot.send(event, fmt::format("已退出群[{}](如果在群里的话)", option));
    } else if (option.starts_with("-blq")) {
      std::string msg{"以下为黑名单qq用户\n"};
      for (const auto &[uid, reason] : botmanage::GetQQBlackList())
        msg += fmt::format("{}:{}\n", uid, reason);
      msg.pop_back();
      bot.send(event, msg);
    } else if (option.starts_with("-blg")) {
      std::string msg{"以下为黑名单群\n"};
      for (const auto &[gid, reason] : botmanage::GetGroupBlackList())
        msg += fmt::format("{}:{}\n", gid, reason);
      msg.pop_back();
      bot.send(event, msg);
    } else if (option.starts_with("-caf")) {
      botmanage::add_friend_flag = !botmanage::add_friend_flag;
      bot.send(event,
               fmt::format("自动同意添加好友状态目前为:{}",
                           botmanage::add_friend_flag ? "启用" : "关闭"));
    } else if (option.starts_with("-cag")) {
      botmanage::add_group_flag = !botmanage::add_group_flag;
      bot.send(event, fmt::format("自动同意入群邀请状态目前为:{}",
                                  botmanage::add_group_flag ? "启用" : "关闭"));
    } else if (option.starts_with("-gf")) {
      if (botmanage::reject_group_force_flag) {
        botmanage::reject_group_force_flag = false;
        bot.send(event,
                 fmt::format("已关闭严格模式,自动同意入群邀请状态目前为:{}",
                             botmanage::add_group_flag ? "启用" : "关闭"));
      } else {
        botmanage::reject_group_force_flag = true;
        botmanage::add_group_flag = false;
        bot.send(event, "已启用严格模式");
      }
    } else
      bot.send(event, hint);
  }
}

}  // namespace module
}  // namespace white