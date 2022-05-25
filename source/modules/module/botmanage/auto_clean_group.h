#pragma once

#include "co/co.h"
#include "event/Registrar.h"
#include "global_config.h"
#include "modules/module/botmanage/botmanage.h"
#include "modules/module_interface.h"
#include "modules/module/botmanage/config.h"
#include "permission/permission.h"
#include "type.h"
#include "utility.h"

namespace white {
namespace module {

class AutoCleanGroup : public Module {
 public:
  AutoCleanGroup()
      : Module("botmanage/botmanage.yml", botmanage::kConfigExample) {
    auto config = LoadConfig();
    botmanage::auto_clean_after = config["清理几天未活跃的群聊"].as<int>();
  }
  virtual void Register() override {
    OnFullmatch({"自动清理"}, make_pair("__group_auto_clean__", "机器人管理"),
                "自动清理过久未说话的群", ACT_InClass(AutoCleanGroup::DoClean),
                permission::SUPERUSER, permission::ALWAYS_ON, true, true);
  }

 private:
  void DoClean(const Event &event, onebot11::ApiBot &bot);
};

inline void AutoCleanGroup::DoClean(const Event &event, onebot11::ApiBot &bot) {
  auto group_list = bot.get_group_list().get();
  auto self_id = event["self_id"].get<QId>();
  unsigned int count = 0;
  for (auto &white : config::SUPERUSERS)
    bot.send_private_msg(white, "已开始自动清理");
  for (auto &group : group_list) {
    auto group_id = group.group_id;
    auto self_info = bot.get_group_member_info(group_id, self_id).get();
    if (datetime::GetTimeStampS() - self_info.last_sent_time >=
        3600 * 24 * botmanage::auto_clean_after) {
      bot.send_group_msg(
          group_id,
          fmt::format("本群已超过{}未使用{}，即将自动退群，如有误请联系维护者",
                      botmanage::auto_clean_after, config::BOT_NAME));
      co::sleep(2000);
      bot.set_group_leave(group_id);
      ++count;
      for (auto &white : config::SUPERUSERS)
        bot.send_private_msg(
            white, fmt::format("已自动退出群{}(超过{}天未使用)", group_id,
                               botmanage::auto_clean_after));
    }
  }
  for (auto &white : config::SUPERUSERS)
    bot.send_private_msg(white, fmt::format("清理结束，共清理{}个群", count));
}

}  // namespace module
}  // namespace white