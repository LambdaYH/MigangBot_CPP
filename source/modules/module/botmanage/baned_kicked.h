#pragma once

#include "event/Registrar.h"
#include "global_config.h"
#include "modules/module_interface.h"
#include "modules/module/botmanage/config.h"
#include "permission/permission.h"
#include "utility.h"

namespace white {
namespace module {

class BandedOrKicked : public Module {
 public:
  BandedOrKicked() : Module() { botmanage::Init(); }
  virtual void Register() override {
    OnNotice("group_decrease", "kick_me",
             make_pair("__handle_kicked__", "机器人管理"),
             "处理被T出事件，被T出后自动将操作者和群加入黑名单",
             ACT_InClass(BandedOrKicked::Handlekicked), permission::NORMAL,
             permission::SUPERUSER, true, false);
  }

 private:
  void HandleBanded(const Event &event,
                    onebot11::ApiBot &bot) { /*暂时不想handle*/
  }
  void Handlekicked(const Event &event, onebot11::ApiBot &bot);
};

inline void BandedOrKicked::Handlekicked(const Event &event,
                                         onebot11::ApiBot &bot) {
  auto group_id = event["group_id"].get<GId>();
  auto operator_id = event["operator_id"].get<QId>();
  auto time_now = datetime::GetCurrentTime();
  auto user_info = bot.get_stranger_info(operator_id).get();
  for (auto &white : config::SUPERUSERS)
    bot.send_private_msg(
        white, fmt::format("{}|已被(Q){}({})踢出(群){}", time_now,
                           user_info.nickname, operator_id, group_id));
  botmanage::AddToGroupBlackList(group_id, "直接踢出群而未使用.dismiss");
  botmanage::AddToQQBlackList(operator_id, "直接踢出群而未使用.dismiss");
  for (auto &white : config::SUPERUSERS)
    bot.send_private_msg(
        white, fmt::format("{}|已将(Q){}({})&(群){}加入黑名单", time_now,
                           user_info.nickname, operator_id, group_id));
}

}  // namespace module
}  // namespace white