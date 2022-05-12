#pragma once

#include "modules/module_interface.h"

#include <unordered_set>

#include <co/co.h>

#include "global_config.h"
#include "message/message_segment.h"
#include "message/utility.h"
#include "modules/module/botmanage/config.h"
#include "type.h"
#include "utility.h"

namespace white {
namespace module {

class GroupInvite : public Module {
 public:
  GroupInvite()
      : Module("botmanage/botmanage.yml", botmanage::kConfigExample),
        config_(LoadConfig()),
        reason_(config_["拒绝理由"].as<std::string>()),
        auto_leave_reason_(config_["自动退群理由"].as<std::string>()) {
    botmanage::add_group_flag = config_["是否同意加群邀请"].as<bool>();
    botmanage::reject_group_force_flag =
        config_["是否自动退出被迫自动入的群"].as<bool>();
  }
  virtual void Register() override {
    OnRequest("group", "invite", "__处理邀请加群__",
              ACT_InClass(GroupInvite::Handle_group_invite), permission::NORMAL,
              permission::SUPERUSER);
    OnNotice("group_increase", "", "__处理群员增加__",
             ACT_InClass(GroupInvite::Handle_group_increase), permission::NORMAL,
             permission::SUPERUSER);
  }

 private:
  void Handle_group_invite(const Event &event, onebot11::ApiBot &bot);
  void Handle_group_increase(const Event &event, onebot11::ApiBot &bot);

 private:
  Config config_;
  std::string reason_;
  std::string auto_leave_reason_;
};

inline void GroupInvite::Handle_group_invite(const Event &event,
                                             onebot11::ApiBot &bot) {
  auto user_id = event["user_id"].get<QId>();
  auto group_id = event["group_id"].get<GId>();
  if (botmanage::add_group_flag) {
    if (botmanage::IsBlackUser(user_id) || botmanage::IsBlackGroup(group_id)) {
      bot.reject(event, fmt::format("黑名单:{}", botmanage::IsBlackUser(user_id)
                                                     ? user_id
                                                     : group_id));
      auto user_info = bot.get_stranger_info(user_id).Ret();
      for (auto white : config::SUPERUSERS)
        bot.send_private_msg(
            white, fmt::format("{}|{}已拒绝加入(群){}，邀请人{}({})(黑名单)",
                               datetime::GetCurrentTime(), config::BOT_NAME,
                               group_id, user_info.nickname, user_id));
    } else {
      bot.approve(event);
      auto user_info = bot.get_stranger_info(user_id).Ret();
      for (auto white : config::SUPERUSERS)
        bot.send_private_msg(
            white, fmt::format("{}|{}新加入(群){}，邀请人{}({})",
                               datetime::GetCurrentTime(), config::BOT_NAME,
                               group_id, user_info.nickname, user_id));
    }
  } else {
    bot.reject(event, reason_);
    auto user_info = bot.get_stranger_info(user_id).Ret();
    for (auto white : config::SUPERUSERS)
      bot.send_private_msg(
          white,
          fmt::format("{}|{}已拒绝加入(群){}，邀请人{}({})(暂停自动入群)",
                      datetime::GetCurrentTime(), config::BOT_NAME, group_id,
                      user_info.nickname, user_id));
  }
}

inline void GroupInvite::Handle_group_increase(const Event &event,
                                               onebot11::ApiBot &bot) {
  auto user_id = event["user_id"].get<QId>();
  auto group_id = event["group_id"].get<GId>();
  auto self_id = event["self_id"].get<QId>();
  if (user_id == self_id) {
    if (botmanage::reject_group_force_flag) {
      bot.send_group_msg(group_id, auto_leave_reason_);
      co::sleep(2000);
      bot.set_group_leave(group_id);
      for (auto white : config::SUPERUSERS)
        bot.send_private_msg(
            white, fmt::format("{}|{}已自动退出(群){}(严格模式)",
                               datetime::GetCurrentTime(), config::BOT_NAME,
                               group_id));
    } else {
      co::sleep(1000);
      bot.send_group_msg(group_id, botmanage::help_msg_group_);
      for (auto white : config::SUPERUSERS)
        bot.send_private_msg(white, fmt::format("{}|{}新加入(群){}，邀请人未知",
                                                datetime::GetCurrentTime(),
                                                config::BOT_NAME, group_id));
    }
  }
}

}  // namespace module
}  // namespace white