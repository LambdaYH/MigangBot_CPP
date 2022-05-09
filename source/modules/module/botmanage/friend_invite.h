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

class FriendInvite : public Module {
 public:
  FriendInvite()
      : Module("botmanage/botmanage.yml", botmanage::kConfigExample),
        config_(LoadConfig()) {
    botmanage::add_friend_flag = config_["是否同意好友邀请"].as<bool>();
  }
  virtual void Register() override {
    RegisterRequest("friend", "", "__处理好友邀请__",
                    ACT(FriendInvite::Handle_friend_invite), permission::NORMAL,
                    permission::SUPERUSER);
  }

 private:
  void Handle_friend_invite(const Event &event, onebot11::ApiBot &bot);

 private:
  Config config_;
};

inline void FriendInvite::Handle_friend_invite(const Event &event,
                                               onebot11::ApiBot &bot) {
  auto user_id = event["user_id"].get<QId>();
  if (botmanage::add_friend_flag) {
    if (botmanage::IsBlackUser(user_id)) {
      auto user_info = bot.get_stranger_info(user_id).Ret();
      for (auto white : config::SUPERUSERS)
        bot.send_private_msg(
            white, fmt::format("{}|好友添加请求，来自{}({}):已拒绝(黑名单)",
                               datetime::GetCurrentTime(), user_info.nickname,
                               user_id));
    } else {
      bot.approve(event);
      co::sleep(2000);
      bot.send_private_msg(user_id, botmanage::help_msg_friend_);
      auto user_info = bot.get_stranger_info(user_id).Ret();
      for (auto white : config::SUPERUSERS)
        bot.send_private_msg(
            white, fmt::format("{}|好友添加请求，来自{}({}):已同意",
                               datetime::GetCurrentTime(), user_info.nickname,
                               user_id));
    }
  } else {
    bot.reject(event, "抱歉，目前暂停加好友了~");
    auto user_info = bot.get_stranger_info(user_id).Ret();
    for (auto white : config::SUPERUSERS)
      bot.send_private_msg(
          white,
          fmt::format("{}|好友添加请求，来自{}({}):已拒绝(暂停自动加好友)",
                      datetime::GetCurrentTime(), user_info.nickname, user_id));
  }
}

}  // namespace module
}  // namespace white